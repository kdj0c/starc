/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include "gamemain.h"

typedef struct {
	char name[64];
	void (*func) (void);
} menuentry_t;

void meOptions(void);
void meExit(void);

#define NB_ENTRY 5

static menuentry_t menu[] = {
	{.name = "SinglePlayer",.func = gmStartSingle,}, {.name = "MultiPlayer",.func = gmStartMulti,}, {.name = "Replay",
																									 .func = gmReplay,}, {.name =
																														  "Options",.func =
																														  meOptions,},
	{.name = "Exit",.func = meExit,}
};

FTGLfont *menufont;
static int cursor = 0;

void meOptions(void) {

}
void meExit(void) {
	exitCleanup();
}

void meDrawMenu(void) {
	int i;
	float h = 600.0;
	float dh;

	dh = h / (NB_ENTRY + 3);

	glTranslatef(100., h - dh * 2, 0.);
	for (i = 0; i < NB_ENTRY; i++) {
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
	SDL_GL_SwapBuffers();
}

void meKeyDown(int key) {
	switch (key) {
	case SDLK_ESCAPE:
		exit(0);
		break;
	case SDLK_DOWN:
		cursor++;
		if (cursor >= NB_ENTRY)
			cursor = NB_ENTRY - 1;
		break;
	case SDLK_UP:
		cursor--;
		if (cursor < 0)
			cursor = 0;
		break;
	case SDLK_RETURN:
		menu[cursor].func();
		break;
	default:
		break;
	}
}

void meLoop(void) {
	SDL_Event ev;
	int done = 0;

	while (!done) {
		if (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT)
				done = 1;
			if (ev.type == SDL_KEYDOWN)
				meKeyDown(ev.key.keysym.sym);
		}
		meDisplayMenu();
		SDL_Delay(50);
	}
}

void meInitMenu(void) {
	menufont = ftglCreateTextureFont("Digeria Normal.ttf");
	ftglSetFontFaceSize(menufont, 30, 30);
}
