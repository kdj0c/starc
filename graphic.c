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
#include <GL/glut.h>
#include <FTGL/ftgl.h>

#include "graphic.h"
#include "pnglite.h"

int grWidth = 100;
int grHeight = 100;
/* TODO remove this global variable */
extern FTGLfont * menufont;

unsigned int grLoadTexture(char * filename) {
	png_t tex;
	unsigned char* data;
	unsigned int textureHandle;

	png_init(0, 0);
	png_open_file_read(&tex, filename);

	data = (unsigned char*) malloc(tex.width * tex.height * tex.bpp);
	png_get_data(&tex, data);

	glGenTextures(1, &textureHandle);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width, tex.height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, data);
	glEnable(GL_TEXTURE_2D);

	png_close_file(&tex);
	free(data);
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
	glColor4f(1.,1.,1.,c);
}

void grBlitLaser(float x, float y, float len, float r, float width) {
	float ax, ay;

	ax = - width * sin(r);
	ay = width * cos(r);
	glBegin(GL_QUADS);
	glTexCoord2f(0., 0.5);
	glVertex2f(x + ax, y + ay);
	glTexCoord2f(0., 0.8);
	glVertex2f(x + len * cos(r) + ax, y + len * sin(r) + ay);
	glTexCoord2f(1., 0.8);
	glVertex2f(x + len * cos(r) - ax, y + len * sin(r) - ay);
	glTexCoord2f(1., 0.5);
	glVertex2f(x - ax, y - ay);
	glEnd();
}

void grBlitSquare(float x, float y, float size) {
	glBegin(GL_QUADS);
	glTexCoord2f(0., 0.);
	glVertex2f(x - size, y);
	glTexCoord2f(0., 1.);
	glVertex2f(x, y + size);
	glTexCoord2f(1., 1.);
	glVertex2f(x + size, y);
	glTexCoord2f(1., 0.);
	glVertex2f(x, y - size);
	glEnd();
}

void grBlitRot(float x, float y, float r, float size) {
	float nr;
	float s;
	float a;
	float b;
	nr = r - M_PI_4;
	s = size * M_SQRT1_2;
	a = s * cos(nr);
	b = s * sin(nr);
	grBlit(x, y, a, b);
}

void grBlit(float x, float y, float a, float b) {
	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 0.f);
	glVertex2f(x + a, y + b);
	glTexCoord2f(0.f, 1.f);
	glVertex2f(x + b, y - a);
	glTexCoord2f(1., 1.);
	glVertex2f(x - a, y - b);
	glTexCoord2f(1., 0.);
	glVertex2f(x - b, y + a);
	glEnd();
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
	yscr = (wu * grHeight) / (4.f * grWidth);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, grWidth, 0, grHeight, 0, 1);
	glScalef(grWidth / (float) wu, grWidth / (float) wu, 1.f);
	glTranslatef(xscr, yscr, 0);
	glRotatef((-r * 180.f) / M_PI + 90.0, 0, 0, 1);
	glTranslatef(-x, -y, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void grReshape(int width, int height) {
	grWidth = width;
	grHeight = height;
	glViewport(0, 0, width, height);
	printf("w,%d, h, %d\n", width, height);
}

void grDrawHUD(float health) {
	char h[16];
	int hea;
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, 800, 0., 600);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(0.0, 1.0, 0.0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	hea = (int) health;
	if (hea < 0)
		hea = 0;
	sprintf(h,"+ %d", hea);
	ftglRenderFont(menufont, h, FTGL_RENDER_ALL);

}

