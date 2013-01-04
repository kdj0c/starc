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

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_image.h>

#include "menu.h"
#include "gamemain.h"
#include "graphic.h"
#include "config.h"

SDL_Joystick *joy = NULL;

void gamepad(void) {
	int numjoy;

	numjoy = SDL_NumJoysticks();
	printf("sdl found %d joy\n", numjoy);
	//for (j = 0; j < numjoy; j++) {
	if (numjoy > 0)
		joy = SDL_JoystickOpen(0);
	printf("joystick #%d, %s opened\n", 0, SDL_JoystickName(0));
	//}
}

void exitCleanup(void) {
	printf("cleanup before exiting\n");
	if (joy)
		SDL_JoystickClose(joy);
	SDL_Quit();
	exit(0);
}

int main(int argc, char *argv[], char *envp[]) {
	grconf_t c;
	SDL_VideoInfo *info;
	int flags;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

	cfReadGraphic(&c);

	//vsync
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	/*	info = SDL_GetVideoInfo();
	 *printf("fullscreen resolution %d %d\n",info->current_w,
	 *info->current_h); */

	flags = SDL_OPENGL;
	if (c.fullscreen)
		flags |= SDL_FULLSCREEN;
	SDL_SetVideoMode(c.width, c.heigh, 32, flags);
	grReshape(c.width, c.heigh);

	srand(1983);

	meInitMenu();

	gamepad();

	meLoop();

	return 0;
}
