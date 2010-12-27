/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <math.h>
#include <string.h>
#include "graphic.h"
#include "ship.h"

shiptype_t shipv2 = {
	.name = "v2",
	.imgfile = "img/v2.png",
	.size = 640,
	.shieldfile = "img/shield.png",
	.shieldsize = 850,
	.maxhealth = 1000,
	.maniability = 0.001,
	.thrust = 0.01,
	.numlaser = 1,
	.laser[0].x = 0,
	.laser[0].y = 310,
	.laser[0].r = 0,
};

shiptype_t * alltype[] = {&shipv2, NULL};

static ship_t * head = NULL;

static void addShip(ship_t * sh) {
	if(head) {
		sh->next = head->next;
		head->next = sh;
	} else {
		head = sh;
		sh->next = NULL;
	}
}

void shLoadShip(void) {
	shiptype_t * sht;

	sht = &shipv2;
	//for(sht = alltype[0];sht != NULL; sht++) {
		sht->tex = grLoadTexture(sht->imgfile);
		sht->shieldtex = grLoadTexture(sht->shieldfile);
	//}
}

ship_t * shCreateShip(char * name, float x, float y, float r) {
	shiptype_t * sht;
	ship_t * newship;

	newship = malloc(sizeof(ship_t));
	memset(newship,0,sizeof(ship_t));
	sht = &shipv2;
//	for(sht = alltype[0];sht != NULL; sht++) {
		if(!strcmp(name,sht->name))
			newship->t = sht;
//	}
	newship->x = x;
	newship->y = y;
	newship->r = r;
	newship->health = newship->t->maxhealth;
	addShip(newship);
	return newship;
}

void shUpdateShips(float dt) {
	ship_t * sh;
	for (sh = head; sh != NULL; sh = sh->next) {
		if(sh->in.direction) {
			sh->r += sh->in.direction * dt * sh->t->maniability;
		}
		if(sh->in.acceleration) {
			sh->dx += sh->t->thrust * dt * cos(sh->r);
			sh->dy += sh->t->thrust * dt * sin(sh->r);
		}
		sh->x += sh->dx;
		sh->y += sh->dy;
	}
}

void shDrawShips(void) {
	ship_t * sh;
	for (sh = head; sh != NULL; sh = sh->next) {
		grSetBlend(sh->t->tex);
		grBlitRot(sh->x,sh->y,sh->r,sh->t->size);
	}
}

