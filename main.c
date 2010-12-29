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

#include "graphic.h"
#include "ship.h"
#include "ai.h"
#include "star.h"
#include "particle.h"

ship_t * player;
ai_t * ai;

void dummy() {

}

void grDraw(int value) {
	static int timebase = 0;
	static int frame = 0;
	int time;
	float fps;

	glutTimerFunc(10,grDraw,0);
	frame++;
	time = glutGet(GLUT_ELAPSED_TIME);

	if (time - timebase > 1000) {
		fps = frame*1000.0/((float)(time-timebase));
		printf("fps %f\n",fps);
		timebase = time;
		frame = 0;
	}

	grChangeview(player->x,player->y,player->r);

	glClear(GL_COLOR_BUFFER_BIT ); //Efface le frame buffer et le Z-buffer
    glColor4f(1.0, 1.0, 1.0, 1.0);
    stUpdate(player->x,player->y);
    stBlit();
	paUpdate(10);
	aiThink(ai);
	shUpdateShips(10);
	shDrawShips();
	glutSwapBuffers();
}

void keyup(unsigned char key, int x, int y) {
	switch(key) {
	case 'c':
		player->in.acceleration = 0;
		break;
	case 'h':
	case 'n':
		player->in.direction = 0;
		break;
	case ' ':
		player->in.fire1 = 0;
	}
}
void keydown(unsigned char key, int x, int y){
	switch(key) {
	case 27:
		exit(0);
	case 'c':
		player->in.acceleration = 1;
		break;
	case 'h':
		player->in.direction = 1;
		break;
	case 'n':
		player->in.direction = -1;
		break;
	case ' ':
		player->in.fire1 = 1;
	}
}

int main(int argc, char *argv[], char *envp[]) {
	int WindowName;

    srand (1983);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	WindowName = glutCreateWindow("test particle");
	glutFullScreen();

	glutReshapeFunc(grReshape);
	glutDisplayFunc(dummy);
	glutKeyboardUpFunc(keyup);
	glutKeyboardFunc(keydown);
	glutSetCursor(GLUT_CURSOR_NONE);
	shLoadShip();
	paInit();
	//player = shCreateShip("v2", 5000,3000,0);
	player = shCreateShip("v2", 0,0,0,0);
	ai = aiCreate(shCreateShip("v1",900,900,0,1),player);
	shCreateShip("v2", 0,900,0,0);
	shCreateShip("v2", 900,0,0,0);
	shCreateShip("v2", -900,0,0,0);
	paExplosion(0,0,5.f,300);
	glutTimerFunc(10,grDraw,0);
	glutMainLoop();
	return 0;
}
