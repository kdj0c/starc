/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <GL/glut.h>
#include <FTGL/ftgl.h>

#include "gamemain.h"

typedef struct {
	char name[64];
	void (*func)(void);
} menu_t;

void meOptions(void);
void meExit(void);

static menu_t menu[] = {{
		.name = "SinglePlayer",
		.func = gmStartSingle,
}, {
		.name = "Multiplayer",
		.func =  gmStartMulti,
}, {
		.name = "Options",
		.func = meOptions,
}, {
		.name = "Exit",
		.func = meExit,
}};

static FTGLfont * menufont;
static int cursor = 0;

void meOptions(void) {

}
void meExit(void) {
	exit(0);
}

void meDrawMenu(void) {
	int i;
	int nbentry = 3;
	float w = 800.0, h = 600.0;
	float dh;

	dh = h / (nbentry + 4);

	glTranslatef(100., h - dh * 2, 0.);
	for (i = 0; i < 4; i++) {
		if (i == cursor)
			glColor3f(1.0, 1.0, 1.0);
		else
			glColor3f(0., 1., 0.);

		ftglRenderFont(menufont, menu[i].name, FTGL_RENDER_ALL);
		glTranslatef(0., -dh, 0.);
	}
}

void meDisplayMenu(void) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, 800, 0., 600);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(0.0, 1.0, 0.0);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	meDrawMenu();
	glutSwapBuffers();
}

void meKeyDown(unsigned char key, int x, int y) {
	switch (key) {
	case 13:
	case ' ':
		/* Launch game or option menu */
		menu[cursor].func();
		break;
	case 27:
		exit(0);
		break;
	default:
		break;
	}

}

void meSpecialDown(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		cursor--;
		if (cursor < 0)
			cursor = 0;
		break;
	case GLUT_KEY_DOWN:
		cursor++;
		if (cursor > 3)
			cursor = 3;
		break;
	default:
		break;
	}
	glutPostRedisplay();

}
void meInitMenu(void) {
	menufont = ftglCreateTextureFont("FreeMono.ttf");
	ftglSetFontFaceSize(menufont, 60, 60);

	glutKeyboardFunc(meKeyDown);
	glutSpecialFunc(meSpecialDown);

}

