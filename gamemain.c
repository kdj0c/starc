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
#include "turret.h"
#include "config.h"
#include "event.h"

ship_t * player = NULL;
static float scale = 1.f;
static int net = 1;
static int gpause = 0;
shin_t pl_in = {0 ,};
float frametime;

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
	evConsumeEvent(time);
	frametime = time;
	if (!player)
		player = shGetByID(0);

	if (gpause && !net) {
		fpstime = time;
		prevtime = time;
		return;
	}

	if (time - fpstime > 1000) {
		fps = frame * 1000.0 / ((float) (time - fpstime));
		printf("fps %f\n", fps);
		fpstime = time;
		frame = 0;
	}

	dt = (float) time - prevtime;
	prevtime = time;
	/* Big Lag don't allow less than 2FPS */
	if (dt > 500.)
		dt = 500.;
	else if (dt <= 0)
		dt = 1.;

	glClear(GL_COLOR_BUFFER_BIT);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	if (player)
		stUpdate(player->pos.p.x, player->pos.p.y);
	else
		stUpdate(0.0, 0.0);

	if(net)
		ntHandleMessage();
	aiThink();
	shUpdateShips(time);
	if(!net) {
		shDetectCollision();
		shUpdateRespawn(dt);
	}
	if (player)
		grChangeview(player->pos.p.x, player->pos.p.y, player->pos.r, scale);
	else
		grChangeview(0.0, 0.0, 0.0, scale);
	stBlit();
	shDrawShips();
	paUpdate(dt);
	if (player) {
		grDrawHUD(player->health);
		shDrawShipHUD(player);
	}
	glutSwapBuffers();
}

void keyup(unsigned char key, int x, int y) {
	switch(key) {
	case 'c':
		pl_in.acceleration = 0;
		break;
	case 'h':
	case 'n':
		pl_in.direction = 0;
		break;
	case ' ':
		pl_in.fire1 = 0;
		break;
	}
	if(net)
		ntSendInput(player);
	evPostTrajEv(&pl_in, 0);
}

void keydown(unsigned char key, int x, int y){
	switch(key) {
	case 27:
		exit(0);
	case 'c':
		pl_in.acceleration = 1;
		break;
	case 'h':
		pl_in.direction = 1;
		break;
	case 'n':
		pl_in.direction = -1;
		break;
	case ' ':
		pl_in.fire1 = 1;
		break;
	case '-':
		scale /= 1.3;
		break;
	case '+':
		scale *= 1.3;
		break;
	case 'p':
		gpause ^= 1;
		break;
	}
	if(net)
		ntSendInput(player);
	evPostTrajEv(&pl_in, 0);
}

void SpecialDown(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		pl_in.acceleration = 1;
		break;
	case GLUT_KEY_LEFT:
		pl_in.direction += 1;
		break;
	case GLUT_KEY_RIGHT:
		pl_in.direction -= 1;
		break;
	default:
		break;
	}
	if(net)
		ntSendInput(player);
	evPostTrajEv(&pl_in, 0);
}

void SpecialUp(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		pl_in.acceleration = 0;
		break;
	case GLUT_KEY_LEFT:
		pl_in.direction -= 1;
		break;
	case GLUT_KEY_RIGHT:
		pl_in.direction += 1;
		break;
	default:
		break;
	}
	if(net)
		ntSendInput(player);
	evPostTrajEv(&pl_in, 0);
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
    make_pos(player, 0., 0., 0.);
    make_pos(mother, 0., 20000., 0.);
    make_pos(ai1, 23000., 0.,  0.);
    make_pos(ai2, 20000., 0., 0.);

	net = 0;
	enterGameMode();
	cfReadGameData();
	tuLoadTurret();
	shLoadShip();
	paInit();

	evPostCreateShip("v2", &pos_player, 0, 0);
	evPostCreateShip("mother1", &pos_mother, 0, 1);

	evPostCreateShip("w1", &pos_ai1, 1, 2);
	evPostCreateShip("w2", &pos_ai2, 1, 3);

/*	player = shCreateShip("v2", 0, 0, 0, 0, 0);
	aiCreate(shCreateShip("mother1", 0, 20000, 0, 0, 1));

	aiCreate(shCreateShip("mother1", 0, 20000, 0, 0, 0));
	aiCreate(shCreateShip("v2", 0, -2000, 0, 0, 0));

	aiCreate(shCreateShip("w1", 10000, 10000, -1, 1, 0));
	aiCreate(shCreateShip("w2", 10000, 2000, -1, 1, 0));
	aiCreate(shCreateShip("w1", 10000, -2000, 0, 1, 0));
	aiCreate(shCreateShip("w1", 10000, -10000, -1, 1, 0));
	aiCreate(shCreateShip("w2", 15000, 10000, -1, 1, 0));
	aiCreate(shCreateShip("w1", 15000, -10000, 0, 1, 0));
	aiCreate(shCreateShip("w1", 15000, 2000, -1, 1, 0));
	aiCreate(shCreateShip("w2", 15000, -2000, -1, 1, 0));
	aiCreate(shCreateShip("w1", 20000, 10000, 0, 1, 0));
	aiCreate(shCreateShip("w1", 20000, -10000, -1, 1, 0));
	aiCreate(shCreateShip("w2", 20000, 2000, -1, 1, 0));
	aiCreate(shCreateShip("w1", 20000, -2000, 0, 1, 0));
	aiCreate(shCreateShip("w1", 25000, -2000, -1, 1, 0));
	aiCreate(shCreateShip("w2", 25000, 2000, -1, 1, 0));
	aiCreate(shCreateShip("w1", 25000, 10000, 0, 1, 0));


	aiCreate(shCreateShip("mother1", -15000, -1800, 0, 1, 0));
	*/
//	evPostEventNow(NULL, 0, ev_newship);
//	evPostEventNow(NULL, 0, ev_newtraj);
	glutTimerFunc(10, grDraw, 0);
}

void gmStartMulti(void) {
	net = 1;
	enterGameMode();

	ntInit();
	cfReadGameData();
	tuLoadTurret();
	shLoadShip();
	paInit();
	ntHandleMessage();
	player = ntCreateLocalPlayer("v2");
	glutTimerFunc(10, grDraw, 0);
}



