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
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
//#include <FTGL/ftgl.h>

#include "graphic.h"
#include "vec.h"
#include "shader.h"

int grWidth = 100;
int grHeight = 100;
/* TODO remove this global variable */
//extern FTGLfont * menufont;

static GLint uniform_pos_off;
static GLint uniform_pos_scal;

static GLuint quad_vbo;
static GLuint quad_vao;
void init_quad(void) {
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

void init_basic_shader(void)
{
    GLuint basic_shader;
	basic_shader = create_programme_from_files ("shaders/basic_tex_vs.glsl", "shaders/basic_tex_fs.glsl");

	uniform_pos_off = glGetUniformLocation (basic_shader, "position_offset");
	uniform_pos_scal = glGetUniformLocation (basic_shader, "position_scaling");

	glUseProgram (basic_shader);
    glUniform3f(uniform_pos_off, 0.0f, 0.0f, 0.0f);
    glUniform3f(uniform_pos_scal, 0.002f, 0.002f, 0.0f);

//	colour_loc = glGetUniformLocation (shader_programme, "inputColour");

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
	glColor4f(1.0, 1.0, 1.0, 1.0);
}

void grSetColor(unsigned int color) {
	glColor4ub((color >> 24) & 0xFF, (color >> 16) & 0xFF, (color >> 8) & 0xFF,
			color & 0xFF);
}
void grSetShadow(float c) {
	glColor4f(1., 1., 1., c);
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
    glBufferSubData(GL_ARRAY_BUFFER, NULL, sizeof(points2), points2);
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
    glBufferSubData(GL_ARRAY_BUFFER, NULL, sizeof(points2), points2);
    glBindVertexArray (quad_vao);
    /* draw points 0-3 from the currently bound VAO with current in-use shader */
	glDrawArrays (GL_TRIANGLE_FAN, 0, 4);

#if 0
	glBegin(GL_QUADS);
	glTexCoord2f(1.f, 0.f);
	glVertex2f(p.x + a, p.y + b);
	glTexCoord2f(1.f, 1.f);
	glVertex2f(p.x + b, p.y - a);
	glTexCoord2f(0., 1.);
	glVertex2f(p.x - a, p.y - b);
	glTexCoord2f(0., 0.);
	glVertex2f(p.x - b, p.y + a);
	glEnd();
#endif
}
void grDrawLine(float x1, float y1, float x2, float y2) {
	glDisable(GL_BLEND);
	glLineWidth(1.f);
	glBegin(GL_LINES);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
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

void grReshape(SDL_Window* window, int width, int height) {
	grWidth = width;
	grHeight = height;
	glViewport(0, 0, width, height);
	SDL_SetWindowSize(window, width, height);
	printf("w,%d, h, %d\n", width, height);
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

