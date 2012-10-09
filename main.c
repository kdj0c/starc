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

int main(int argc, char *argv[], char *envp[]) {
	grconf_t c;
	SDL_VideoInfo *info;
	int flags;

	SDL_Init(SDL_INIT_VIDEO);

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

	meLoop();

	return 0;
}
