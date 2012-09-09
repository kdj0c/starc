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
#include "weapon.h"

ship_t * player = NULL;
static float scale = 1.f;
int g_net = 1;
static int gpause = 0;
shin_t pl_in = {0 ,};
float frametime;

void dummy() {

}

void grDraw(int value) {
	static int fpstime = 0;
	static int frame = 0;
	int time;
	float fps;

	glutTimerFunc(10, grDraw, 0);
	frame++;
	time = glutGet(GLUT_ELAPSED_TIME);
	frametime = time;
	if (!player)
		player = shGetByID(0);

	if (gpause && !g_net) {
		fpstime = time;
		return;
	}

	if (time - fpstime > 1000) {
		fps = frame * 1000.0 / ((float) (time - fpstime));
		printf("fps %f\n", fps);
		fpstime = time;
		frame = 0;
	}

	glClear(GL_COLOR_BUFFER_BIT);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	if (player)
		stUpdate(player->pos.p.x, player->pos.p.y);
	else
		stUpdate(0.0, 0.0);

	if(g_net)
		ntHandleMessage();
	aiThink(time);

    evConsumeEvent(time);
	if(!g_net) {
	    shUpdateShips(time);
		shDetectCollision(time);
		weUpdate(time);
        evConsumeEvent(time);
	}
	if (player)
		grChangeview(player->pos.p.x, player->pos.p.y, player->pos.r, scale);
	else
		grChangeview(0.0, 0.0, 0.0, scale);
	stBlit();
	shDrawShips(time);
	weDraw(time);
	paDraw(time);
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
    default:
        return;
	}
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
    default:
        return;
	}
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
		return;
	}
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
		return;
	}
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
    make_pos(mother, 0., 10000., 0.);
    make_pos(ai1, 5000., 0.,  0.);
    make_pos(ai2, 5000., 3000., 0.);

	g_net = 0;
	enterGameMode();
	cfReadGameData();
	shLoadShip();
	paInit();
	weInit();

	evPostCreateShip("v2", &pos_player, 0, 0, pl_local);
	evPostCreateShip("mother1", &pos_mother, 0, 1, pl_ai);

	evPostCreateShip("w1", &pos_ai1, 1, 2, pl_ai);
	evPostCreateShip("w2", &pos_ai2, 1, 3, pl_ai);

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
    make_pos(player, 0., 0., 0.);
	g_net = 1;
	enterGameMode();

	ntInit();
	cfReadGameData();
	shLoadShip();
	paInit();
	weInit();
	ntHandleMessage();
    evPostCreateShip("v2", &pos_player, 0, 0, pl_local);
	glutTimerFunc(10, grDraw, 0);
}



