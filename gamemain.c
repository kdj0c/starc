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

#define GL_GLEXT_PROTOTYPES
#ifdef _WIN32
#define GLEW_STATIC
#include <GL/glew.h>
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>

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
#include "gametime.h"
#include "save.h"

ship_t *player = NULL;
static float scale = 1.f;
int g_net = 1;
static int gpause = 0;
shin_t pl_in = { 0, };

int kleft = 0;
int kright = 0;

#ifndef NETWORK
int curid = 0;

int ntGetId(void) {
	return curid++;
}
#endif

void dummy() {

}

void gmDraw(void) {
	static int fpstime = 0;
	static int frame = 0;
	int time;
	float fps;

	frame++;
	time = gtGetTime();
	if (!player)
		player = shGetPlayer();

	if (gpause && !g_net) {
		fpstime = time;
		return;
	}

	if (time - fpstime > 1000) {
		fps = frame * 1000.0 / ((float) (time - fpstime));
		printf("fps %f\n", fps);
		fpstime = time;
		frame = 0;
		if (g_net)
			ntSendPing();
	}

	glClear(GL_COLOR_BUFFER_BIT);

	if (player)
		stUpdate(player->pos.p.x, player->pos.p.y);
	else
		stUpdate(0.0, 0.0);

	if (player)
		grChangeview(player->pos.p.x, player->pos.p.y, player->pos.r, scale);
	else
		grChangeview(0.0, 0.0, 0.0, scale);
	stDraw();

	shDrawShips(time);
	paDrawExplosion(time);

	weDraw(time);

	shDrawShields(time);
	paDraw(time);

	grSwap();
}

static void sendkey(void) {
	if (!player)
		return;

	if (kleft && kright)
		pl_in.direction = 0.;
	else if (kleft)
		pl_in.direction = 1.;
	else if (kright)
		pl_in.direction = -1.;
	else
		pl_in.direction = 0.;

	evPostTrajEv(&pl_in, player->netid);
}

void keyup(int key) {
	switch (key) {
	case SDLK_c:
	case SDLK_UP:
		pl_in.acceleration = 0;
		break;
	case SDLK_LEFT:
	case SDLK_a:
		kleft = 0;
		break;
	case SDLK_RIGHT:
	case SDLK_n:
		kright = 0;
		break;
	case SDLK_SPACE:
		pl_in.fire1 = 0;
		break;
	default:
		return;
	}
	sendkey();
}

void keydown(int key) {
	switch (key) {
	case SDLK_c:
	case SDLK_UP:
		pl_in.acceleration = 1;
		break;
	case SDLK_LEFT:
	case SDLK_a:
		kleft = 1;
		break;
	case SDLK_RIGHT:
	case SDLK_n:
		kright = 1;
		break;
	case SDLK_SPACE:
		pl_in.fire1 = 1;
		break;
	case SDLK_MINUS:
		scale /= 1.3;
		return;
	case SDLK_PLUS:
	case SDLK_EQUALS:
		scale *= 1.3;
		return;
	case SDLK_ESCAPE:
		exit(0);
		return;
	default:
		return;
	}
	sendkey();
}

void joyAxisMove(int axis, int value) {
	if (axis == 0) {
		pl_in.direction = -((float) value) / 32768.f;
	} else if (axis == 1) {
		pl_in.acceleration = -((float) value) / 32768.f;
	}
	evPostTrajEv(&pl_in, player->netid);
}

void joyButtonDown(int button) {
	pl_in.fire1 = 1;
	evPostTrajEv(&pl_in, player->netid);
}

void joyButtonUp(int button) {
	pl_in.fire1 = 0;
	evPostTrajEv(&pl_in, player->netid);
}

void enterGameMode(void) {
	SDL_ShowCursor(SDL_DISABLE);
}

void gmEngineLoop(void) {
	float time;

	if (g_net)
		ntHandleMessage();
	time = gtGetTime();
	aiThinkAll(time);
	evConsumeEvent(time);
	shUpdateLocal(time);
	shUpdateShips(time);
	shDetectCollision(time);
	weUpdate(time);
	evConsumeEvent(time);
}

void gmGetEvent(void) {
	SDL_Event ev;

	while (SDL_PollEvent(&ev)) {
		if (ev.type == SDL_QUIT)
			exit(0);
		if (ev.type == SDL_KEYDOWN)
			keydown(ev.key.keysym.sym);
		if (ev.type == SDL_KEYUP)
			keyup(ev.key.keysym.sym);
		if (ev.type == SDL_JOYAXISMOTION)
			joyAxisMove(ev.jaxis.axis, ev.jaxis.value);
		if (ev.type == SDL_JOYBUTTONDOWN)
			joyButtonDown(ev.jbutton.button);
		if (ev.type == SDL_JOYBUTTONUP)
			joyButtonUp(ev.jbutton.button);

		if (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_RESIZED)
			grReshape(ev.window.data1, ev.window.data2);
	}
}

void gmLoop(void) {
	int done = 0;

	while (!done) {
		gmGetEvent();
		gmEngineLoop();
		gmDraw();
		SDL_Delay(1);
	}
}

void gmStartSingle(void) {
	make_pos(player, 0., 0., 0.);
	make_pos(ai1, -2000., 0., 0.);
	make_pos(ai2, 0., 2000., 0.);
	make_pos(ai3, 0., -2000., 0.);
	make_pos(ai4, 20000., 0., M_PI);
	make_pos(ai5, 15000., 0., 0.);
	make_pos(ai6, 15000., 2000., 0.);
	make_pos(ai7, 15000., -2000., 0.);

	g_net = 0;
	gtInit();
	enterGameMode();
	cfReadAtlasData();
	cfReadGameData();
	paInit();
	weInit();

	saInit("replay1.rep");
	evPostCreateShip("Red5", &pos_player, 0, ntGetId(), pl_local);
	evPostCreateShip("stationRed1", &pos_ai1, 0, ntGetId(), pl_ai);
	evPostCreateShip("Red1", &pos_ai2, 0, ntGetId(), pl_ai);
	evPostCreateShip("Red3", &pos_ai3, 0, ntGetId(), pl_ai);
	evPostCreateShip("stationRed1", &pos_ai4, 1, ntGetId(), pl_ai);
	evPostCreateShip("Green2", &pos_ai5, 1, ntGetId(), pl_ai);
	evPostCreateShip("Green1", &pos_ai6, 1, ntGetId(), pl_ai);
	evPostCreateShip("Green2", &pos_ai7, 1, ntGetId(), pl_ai);

	grInitQuad();
	grInitShader();
	gmLoop();
}

void gmStartMulti(void) {
	make_pos(player, 0., 0., 0.);
	g_net = 1;
	enterGameMode();
	gtInit();

	ntClientInit();
	cfReadGameData();
	paInit();
	weInit();
	ntHandleMessage();
	evPostCreateShip("v2", &pos_player, 0, ntGetId(), pl_local);

	gmLoop();
}

void gmReplay(void) {

	g_net = 0;
	gtInit();
	enterGameMode();
	cfReadGameData();
	paInit();
	weInit();

	saReplay("replay1.rep");
	gmLoop();
}
