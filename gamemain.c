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

#include "graphic.h"
#include "ship.h"
#include "ai.h"
#include "star.h"
#include "particle.h"
#include "network.h"

ship_t * player;
static float scale = 1.f;
static int net = 1;

void dummy() {

}

void grDraw(int value) {
	static int fpstime = 0;
	static int frame = 0;
	static int prevtime = 0;
	int time;
	float fps;
	float dt;

	glutTimerFunc(10, grDraw, 0);
	frame++;
	time = glutGet(GLUT_ELAPSED_TIME);

	if (time - fpstime > 1000) {
		fps = frame * 1000.0 / ((float) (time - fpstime));
		printf("fps %f\n", fps);
		fpstime = time;
		frame = 0;
	}
	dt = (float) time - prevtime;
	prevtime = time;

	glClear(GL_COLOR_BUFFER_BIT);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	stUpdate(player->x, player->y);
	if(net)
		ntHandleMessage();
	aiThink();
	shUpdateShips(dt);
	if(!net) {
		shDetectCollision();
		shUpdateRespawn(dt);
	}
	grChangeview(player->x, player->y, player->r, scale);
	stBlit();
	paUpdate(dt);
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
	if(net)
		ntSendInput(player);
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
		break;
	case '-':
		scale /= 1.3;
		break;
	case '+':
		scale *= 1.3;
		break;
	}
	if(net)
		ntSendInput(player);
}

void SpecialDown(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		player->in.acceleration = 1;
		break;
	case GLUT_KEY_LEFT:
		player->in.direction = 1;
		break;
	case GLUT_KEY_RIGHT:
		player->in.direction = -1;
		break;
	default:
		break;
	}
	if(net)
		ntSendInput(player);
}

void SpecialUp(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		player->in.acceleration = 0;
		break;
	case GLUT_KEY_LEFT:
		player->in.direction = 0;
		break;
	case GLUT_KEY_RIGHT:
		player->in.direction = 0;
		break;
	default:
		break;
	}
	if(net)
		ntSendInput(player);
}

void enterGameMode(void) {
	glutIgnoreKeyRepeat(1);
	glutKeyboardUpFunc(keyup);
	glutKeyboardFunc(keydown);
	glutSpecialFunc(SpecialDown);
	glutSpecialUpFunc(SpecialUp);
	glutSetCursor(GLUT_CURSOR_NONE);
	glutDisplayFunc(dummy);
}

void gmStartSingle(void) {
	net = 0;
	enterGameMode();

	shLoadShip();
	paInit();
	player = shCreateShip("v2", 0, 0, 0, 0, 0);
	aiCreate(shCreateShip("v1", 10000, 0, -1, 1, 0));
	aiCreate(shCreateShip("v1", 10000, 900, -1, 1, 0));
	aiCreate(shCreateShip("v2", 0, 900, 0, 0, 0));
	aiCreate(shCreateShip("v2", 0, -900, 0, 0, 0));
	aiCreate(shCreateShip("v1", -10000, -900, 0, 1, 0));
	aiCreate(shCreateShip("v1", -10000, -1800, 0, 1, 0));
	aiCreate(shCreateShip("v2", 0, 50000, 0, 0, 0));

	glutTimerFunc(10, grDraw, 0);
}

void gmStartMulti(void) {
	net = 1;
	enterGameMode();

	ntInit();
	shLoadShip();
	paInit();
	ntHandleMessage();
	player = ntCreateLocalPlayer("v2");
	glutTimerFunc(10, grDraw, 0);
}



