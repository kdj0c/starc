/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define GL_GLEXT_PROTOTYPES
#include <epoxy/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>

#include "graphic.h"
#include "vec.h"
#include "shader.h"

static int grWidth = 100;
static int grHeight = 100;
static SDL_Window* grwindow;

/* TODO remove this global variable */
//extern FTGLfont * menufont;

static GLint uniform_pos_off;
static GLint uniform_pos_scal;
static GLint uniform_colour;

void grInit (void) {
	SDL_GLContext glContext;
    const GLubyte* renderer;
    const GLubyte* version;

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
                                0, 0,
                                SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_OPENGL);
	/*
							  , SDL_WINDOWPOS_CENTERED
							  , SDL_WINDOWPOS_CENTERED
							  , g_gl_width
							  , g_gl_height
							  , SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);*/
	if (!grwindow) {
		fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
		exit(1);
	}

	glContext = SDL_GL_CreateContext(grwindow);
	if (!glContext) {
		fprintf(stderr, "Failed to create OpenGL context: %s\n", SDL_GetError());
		exit(1);
	}

    /* get version info */
    renderer = glGetString (GL_RENDERER); /* get renderer string */
    version = glGetString (GL_VERSION); /* version as a string */
    printf ("Renderer: %s\n", renderer);
    printf ("OpenGL version supported %s\n", version);
}

static GLuint quad_vbo;
static GLuint quad_vao;
void grInitQuad(void) {
	GLfloat points[] = {
		 0.0f,	0.0f,	0.0f,
		 0.5f,  0.0f,	0.0f,
		 0.5f,  0.5f,	0.0f,
		 0.0f,  0.5f,   0.0f
	};

	GLfloat texcoords[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};
	glGenBuffers (1, &quad_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, quad_vbo);
	glBufferData (GL_ARRAY_BUFFER, 12 * sizeof (GLfloat), points, GL_DYNAMIC_DRAW);

	GLuint texcoords_vbo;
	glGenBuffers (1, &texcoords_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, texcoords_vbo);
	glBufferData (GL_ARRAY_BUFFER, 8 * sizeof (GLfloat), texcoords, GL_STATIC_DRAW);


	glGenVertexArrays (1, &quad_vao);
	glBindVertexArray (quad_vao);

	glBindBuffer (GL_ARRAY_BUFFER, quad_vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer (GL_ARRAY_BUFFER, texcoords_vbo);
	glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray (0);
	glEnableVertexAttribArray (1);
}

void grInitShader(void)
{
    GLuint basic_shader;
	basic_shader = create_programme_from_files ("shaders/basic_tex_vs.glsl", "shaders/basic_tex_fs.glsl");

	uniform_pos_off = glGetUniformLocation (basic_shader, "position_offset");
	uniform_pos_scal = glGetUniformLocation (basic_shader, "position_scaling");
	uniform_colour = glGetUniformLocation (basic_shader, "add_colour");

	glUseProgram (basic_shader);
    glUniform3f(uniform_pos_off, 0.0f, 0.0f, 0.0f);
    glUniform3f(uniform_pos_scal, 0.002f, 0.002f, 0.0f);
    glUniform4f(uniform_colour, 1.0f, 1.0f, 1.0f, 1.0f);
}


unsigned int grLoadTexture(char * filename) {
	unsigned int textureHandle;
	SDL_Surface *sdlsurf;

	sdlsurf = IMG_Load(filename);

	glGenTextures(1, &textureHandle);
	glActiveTexture (GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sdlsurf->w, sdlsurf->h, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, sdlsurf->pixels);
	return textureHandle;
}

void grSetBlendAdd(unsigned int text) {
	glBindTexture(GL_TEXTURE_2D, text);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void grSetBlend(unsigned int text) {
	glBindTexture(GL_TEXTURE_2D, text);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUniform4f(uniform_colour, 1.0, 1.0, 1.0, 1.0);
}

#define u8_to_float(c) (((float) (c)) / 255.0f)

void grSetColor(unsigned int color) {
    float r,g,b,a;

    r = u8_to_float((color >> 24) & 0xFF);
    g = u8_to_float((color >> 16) & 0xFF);
    b = u8_to_float((color >> 8) & 0xFF);
    a = u8_to_float(color & 0xFF);

    glUniform4f(uniform_colour, r, g, b, a);
}
void grSetShadow(float c) {
    glUniform4f(uniform_colour, 1.0, 1.0, 1.0, c);
}

void grBlitLaser(float x, float y, float len, float r, float width) {
	float ax, ay, bx, by;

	ax = -width * sin(r);
	ay = width * cos(r);
	bx = len * cos(r);
	by = len * sin(r);

    GLfloat points2[] = {
		 x + ax, y + ay,	0.0f,
		 x + ax + bx, y + ay + by,	0.0f,
		 x - ax + bx, y - ay + by,	0.0f,
		 x - ax, y - ay,   0.0f
	};

    glBindBuffer (GL_ARRAY_BUFFER, quad_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points2), points2);
    glBindVertexArray (quad_vao);
    /* draw points 0-3 from the currently bound VAO with current in-use shader */
	glDrawArrays (GL_TRIANGLE_FAN, 0, 4);
}

void grBlitSquare(vec_t p, float size) {
    grBlit(p, size, 0.f);
}

void grBlitRot(vec_t p, float r, float size) {
	float nr;
	float s;
	float a;
	float b;
	nr = r + M_PI_4;
	s = size * M_SQRT1_2;
	a = s * cos(nr);
	b = s * sin(nr);
	grBlit(p, a, b);
}

void grBlit(vec_t p, float a, float b) {
    GLfloat points2[] = {
		 p.x + a, p.y + b,	0.0f,
		 p.x + b, p.y - a,	0.0f,
		 p.x - a, p.y - b,	0.0f,
		 p.x - b, p.y + a,   0.0f
	};

    glBindBuffer (GL_ARRAY_BUFFER, quad_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points2), points2);
    glBindVertexArray (quad_vao);
    /* draw points 0-3 from the currently bound VAO with current in-use shader */
	glDrawArrays (GL_TRIANGLE_FAN, 0, 4);
}

void grChangeview(float x, float y, float r, float scale) {
	float xscr;
	float yscr;
	float wu;

	wu = WIDTH_UNIT / scale;

	xscr = wu / 2.f;
	/* coordinate tricks : we want width to be 10000 unit */
	yscr = (xscr * grHeight) / ((float)grWidth);

    glUniform3f(uniform_pos_off, -x, -y, 0.0f);
    glUniform3f(uniform_pos_scal, 1.0f/xscr, 1.0f/yscr, 0.0f);
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

