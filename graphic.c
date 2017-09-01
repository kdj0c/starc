/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define GL_GLEXT_PROTOTYPES

#ifdef _WIN32
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>

#include "graphic.h"
#include "vec.h"
#include "shader.h"

// maximum number of packed sprite draw
#define GR_MAX 2000

static int grWidth = 100;
static int grHeight = 100;
static SDL_Window *grwindow;

static GLint uniform_pos_off;
static GLint uniform_pos_scal;
static GLuint atlasTexture;

/* Arrays of vertex, texture coordinates, and colors for batch draw */
static float *lvert;
static float *ltc;
static unsigned int *lcolor;
static int indices[GR_MAX];
static int counts[GR_MAX];
static int count;

void grInit(grconf_t *c) {
	SDL_GLContext glContext;
	const GLubyte *renderer;
	const GLubyte *version;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG, GL_TRUE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	/* Do double buffering in GL */
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	/* Create a new window with an OpenGL surface */
	grwindow = SDL_CreateWindow("starc",
								SDL_WINDOWPOS_UNDEFINED,
								SDL_WINDOWPOS_UNDEFINED,
								c->width, c->heigh, SDL_WINDOW_OPENGL | (c->fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));

	if (!grwindow) {
		fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
		exit(1);
	}

	grWidth = c->width;
	grHeight = c->heigh;

	glContext = SDL_GL_CreateContext(grwindow);
	if (!glContext) {
		fprintf(stderr, "Failed to create OpenGL context: %s\n", SDL_GetError());
		exit(1);
	}
#ifdef _WIN32
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		printf("Glew Iinit Error: %s\n", glewGetErrorString(err));
		exit(1);
	}
#endif
	/* get version info */
	renderer = glGetString(GL_RENDERER);	/* get renderer string */
	version = glGetString(GL_VERSION);	/* version as a string */
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);
}

static GLuint quad_vbo;
static GLuint quad_vao;
static GLuint texcoords_vbo;
static GLuint color_vbo;

static GLuint color;

void grInitQuad(void) {
	int i;

	glGenVertexArrays(1, &quad_vao);
	glBindVertexArray(quad_vao);

	glGenBuffers(1, &quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, GR_MAX * 8 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &texcoords_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, texcoords_vbo);
	glBufferData(GL_ARRAY_BUFFER, GR_MAX *8 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
	glBufferData(GL_ARRAY_BUFFER, GR_MAX *4 * sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, NULL);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	atlasTexture = grLoadTextureArray();
	glEnable(GL_BLEND);

	lvert = malloc(GR_MAX * 8 * sizeof(float));
	ltc = malloc(GR_MAX * 8 * sizeof(float));
	lcolor = malloc(GR_MAX * 4 * sizeof(unsigned int));
	for (i = 0; i < GR_MAX; i++) {
		indices[i] = 4 * i;
		counts[i] = 4;
	}
	count = 0;
	grSetTextureIndex(0);
}

void grInitShader(void) {
	GLuint basic_shader;
	basic_shader = create_programme_from_files("shaders/basic_tex_vs.glsl", "shaders/basic_tex_fs.glsl");

	uniform_pos_off = glGetUniformLocation(basic_shader, "position_offset");
	uniform_pos_scal = glGetUniformLocation(basic_shader, "position_scaling");

	glUseProgram(basic_shader);
	glUniform2f(uniform_pos_off, 0.0f, 0.0f);
	glUniform2f(uniform_pos_scal, 0.002f, 0.002f);
}

void grInvertRedGreen(void *surface, int w, int h) {
	unsigned char * p;
	int i;
	unsigned char tmp;

	p = surface;

	for (i = 0; i < w * h; i++) {
		if (p[4 * i] > p[ 4 * i + 1] && p[4 * i] > p[ 4 * i + 2] &&
			p[ 4 * i + 1] / 8 == p[ 4 * i + 2] / 8) {

			tmp = p[4 * i + 1];
			p[4 * i + 1] = p[4 * i];
			p[4 * i] = tmp;
			}
	}
}

void grLoadTexture(const char *filename, int i) {
	SDL_Surface *sdlsurf;
	GLuint err;

	sdlsurf = IMG_Load(filename);
	if (!sdlsurf) {
		printf("cannot load texture %s\n", filename);
		return;
	}
	//grInvertRedGreen(sdlsurf->pixels, sdlsurf->w, sdlsurf->h);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,	//Mipmap number
					0, 0, i, sdlsurf->w, sdlsurf->h, 1,	//width, height, depth
					GL_RGBA,	//format
					GL_UNSIGNED_BYTE,	//type
					sdlsurf->pixels);	//pointer to data

	err = glGetError();
	if (err)
		printf("GL Error %d when loading texture\n", err);
}

unsigned int grLoadTextureArray(void) {
	unsigned int textureHandle;

	glGenTextures(1, &textureHandle);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureHandle);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1,	// mipmap level
				   GL_RGBA8,	//Internal format
				   4096, 4096,	//width,height
				   5);	//Number of layers

	grLoadTexture("img/atlas.png", 0);
	grLoadTexture("img/explosion1.png", 1);
	grLoadTexture("img/explosion2.png", 2);
	grLoadTexture("img/explosion3.png", 3);
	grLoadTexture("img/explosion4.png", 4);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	return textureHandle;
}

void grSetTextureIndex(int index) {
	glUniform1i(1, index);
}

void grSetBlendAdd(void) {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void grSetBlend(void) {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	color = 0xFFFFFFFF;
}

void grBatchAddRot(vec_t p, float r, texc_t *tex, unsigned int c) {
	vec_t h2;
	vec_t w2;
	int o = 8 * count;

	h2 = vangle(tex->h, r);
	w2 = vangle(tex->w, r);

	lvert[o]     = p.x + h2.x - w2.y;
	lvert[o + 1] = p.y + h2.y + w2.x;
	lvert[o + 2] = p.x + h2.x + w2.y;
	lvert[o + 3] = p.y + h2.y - w2.x;
	lvert[o + 4] = p.x - h2.x + w2.y;
	lvert[o + 5] = p.y - h2.y - w2.x;
	lvert[o + 6] = p.x - h2.x - w2.y;
	lvert[o + 7] = p.y - h2.y + w2.x;

	memcpy(ltc + o, tex->texc, 8 * sizeof(float));

	o = 4 * count;
	lcolor[o] = c;
	lcolor[o + 1] = c;
	lcolor[o + 2] = c;
	lcolor[o + 3] = c;

	count++;
}

void grBatchAdd(vec_t p, float a, float b, texc_t *tex, unsigned int c) {
	int o = 8 * count;

	lvert[o]     = p.x + a;
	lvert[o + 1] = p.y + b;
	lvert[o + 2] = p.x + b;
	lvert[o + 3] = p.y - a;
	lvert[o + 4] = p.x - a;
	lvert[o + 5] = p.y - b;
	lvert[o + 6] = p.x - b;
	lvert[o + 7] = p.y + a;

	memcpy(ltc + o, tex->texc, 8 * sizeof(float));

	o = 4 * count;
	lcolor[o] = c;
	lcolor[o + 1] = c;
	lcolor[o + 2] = c;
	lcolor[o + 3] = c;

	count++;
}

void grBatchDraw(void) {

	glBindVertexArray(quad_vao);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, count * 8 * sizeof(float), lvert);
	glBindBuffer(GL_ARRAY_BUFFER, texcoords_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, count * 8 * sizeof(float), ltc);
	glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, count * 4 * sizeof(unsigned int), lcolor);

	/* draw points 0-3 from the currently bound VAO with current in-use shader */
	glMultiDrawArrays(GL_TRIANGLE_FAN, indices, counts, count);

	count = 0;
}

void grChangeview(float x, float y, float r, float scale) {
	float xscr;
	float yscr;
	float wu;

	wu = WIDTH_UNIT / scale;

	xscr = wu / 2.f;
	/* coordinate tricks : we want width to be 10000 unit */
	yscr = (xscr * grHeight) / ((float) grWidth);

	glUniform2f(uniform_pos_off, -x, -y);
	glUniform2f(uniform_pos_scal, 1.0f / xscr, 1.0f / yscr);
}

void grReshape(int width, int height) {
	grWidth = width;
	grHeight = height;
	glViewport(0, 0, width, height);
	SDL_SetWindowSize(grwindow, width, height);
	printf("Reshape w,%d, h, %d\n", width, height);
}

void grSwap(void) {
	SDL_GL_SwapWindow(grwindow);
}

void grDrawHUD(float health) {
#if 0
	char h[16];
	int ih;
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0., 800, 0., 600);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(0., 1., 0.);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	ih = (int) health;
	if (ih < 0)
		ih = 0;
	sprintf(h, "+ %d", ih);
	ftglRenderFont(menufont, h, FTGL_RENDER_ALL);

	/* new projection for radar */
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0., 800., 0., 600., 0., 1.);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
#endif

}
