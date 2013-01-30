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
/*#include "menu.h"
#include "gamemain.h"
#include "graphic.h"
#include "config.h"
*/
//#include "config.h"

void Render(void) {

glClear(GL_COLOR_BUFFER_BIT);
glBegin(GL_TRIANGLES);
glColor3f(0.,0.,1.);
glVertex3f(0.,0.5,0.);
glColor3f(0.,1.,1.);
glVertex3f(0.5,0.5,0.);
glColor3f(1.,0.,1.);
glVertex3f(1.,0.5,0.);
glEnd();

SDL_GL_SwapBuffers();


}
int main(int argc, char *argv[], char *envp[]) {
    int done = 0;
    SDL_Event ev;
    SDL_Surface *testimg;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_SetVideoMode(600,600,32, SDL_OPENGL);
	//vsync
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    testimg = IMG_Load("img/v1.png");
    if (testimg)
        printf("Yes ca marche\n");
    else
        printf("oh non ca marche pas\n");



    while (!done) {
        SDL_PollEvent(&ev);
        if (ev.type == SDL_QUIT ||
            ev.type == SDL_KEYDOWN)
                done = 1;

        Render();
    }

    SDL_Quit();

	return 0;
}
