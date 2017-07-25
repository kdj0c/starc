/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define GL_GLEXT_PROTOTYPES

#include <SDL2/SDL.h>

#include "gamemain.h"
#include "graphic.h"

//testing only
#include "parse.h"

#include "config.h"

SDL_Joystick *joy = NULL;

void gamepad(void) {
	int numjoy;

	numjoy = SDL_NumJoysticks();
	printf("sdl found %d joy\n", numjoy);
	//for (j = 0; j < numjoy; j++) {
	if (numjoy > 0) {
		joy = SDL_JoystickOpen(0);
		printf("joystick #%d, %s opened\n", 0, SDL_JoystickName(0));
	}
	//}
}

void exitCleanup(void) {
	printf("cleanup before exiting\n");
	if (joy)
		SDL_JoystickClose(joy);
	SDL_Quit();
	exit(0);
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow ) {
#else
int main(int argc, char *argv[]) {
#endif // _WIN32
	grconf_t c;

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK | SDL_INIT_TIMER)<0) {
		  fprintf(stderr, "Failed to initialise SDL: %s\n", SDL_GetError());
		  exit(1);
	}

	atexit(exitCleanup);


	grInit();
	cfReadGraphic(&c);
	gmStartSingle();
	return 0;
}
