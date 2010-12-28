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

shiptype_t shipv1 = {
	.name = "v1",
	.imgfile = "img/v1.png",
	.size = 500,
	.shieldfile = "img/shield.png",
	.shieldsize = 650,
	.maxhealth = 800,
	.maniability = 0.001,
	.thrust = 0.001,
	.numlaser = 2,
	.laser[0].x = 100,
	.laser[0].y = 310,
	.laser[0].r = 0,
	.laser[1].x = -100,
	.laser[1].y = 310,
	.laser[1].r = 0,
};

shiptype_t shipv2 = {
	.name = "v2",
	.imgfile = "img/v2.png",
	.size = 640,
	.shieldfile = "img/shield.png",
	.shieldsize = 850,
	.maxhealth = 1000,
	.maniability = 0.001,
	.thrust = 0.001,
	.numlaser = 2,
	.laser[0].x = 0,
	.laser[0].y = 300,
	.laser[0].r = 0,
	.laser[1].x = 0,
	.laser[1].y = -300,
	.laser[1].r = 0,

};

shiptype_t * alltype[] = {&shipv1,&shipv2, NULL};

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
	int i;

	for(i =0; i < 2; i++) {
		sht = alltype[i];
		sht->tex = grLoadTexture(sht->imgfile);
		sht->shieldtex = grLoadTexture(sht->shieldfile);
	}
}

ship_t * shCreateShip(char * name, float x, float y, float r, int team) {
	shiptype_t * sht;
	ship_t * newship;
	int i;

	newship = malloc(sizeof(ship_t));
	memset(newship,0,sizeof(ship_t));
	for(i =0; i < 2; i++) {
		sht = alltype[i];
		if(!strcmp(name,sht->name))
			newship->t = sht;
	}
	newship->x = x;
	newship->y = y;
	newship->r = r;
	newship->team = team;
	newship->health = newship->t->maxhealth;
	addShip(newship);
	return newship;
}

void shDamage(ship_t * sh, float dg) {
	sh->health -= dg;
	sh->drawshield = 500;
}

void firelaser(ship_t * sh, laser_t * las) {
	float x, y, r, len;
	ship_t * en;
	x = sh->x + las->x * cos(sh->r) + las->y * sin(sh->r);
	y = sh->y + las->x * sin(sh->r) - las->y * cos(sh->r);
	r = sh->r + las->r;
	len = 5000;
	for (en = head; en != NULL; en = en->next) {
		float dx, dy, tx, ty, s;
		if (en == sh)
			continue;
		dx = en->x - x;
		dy = en->y - y;
		tx = dx * cos(r) + dy * sin(r);
		ty = dx * sin(r) - dy * cos(r);
		s = en->t->shieldsize / 2.f;
		if (tx < 5000 && ty > -s && ty < s) {
			len = tx - sqrt(s * s - ty * ty);
			if (len < 5000) {
				shDamage(en, 1.f);
			}
		}
	}
	grDrawLine(x,y,x + len * cos(r),y + len * sin(r));
}

void shCollide(ship_t * sh, ship_t * en, float dx, float dy) {
	float k;
	float m1, m2;

	m1 = sh->t->size;
	m2 = en->t->size;
	k = 2 * (dx * (sh->dx - en->dx) + dy * (sh->dy - en->dy));
	k /= (dx * dx + dy * dy);
	k *= m1 * m2 / (m1 + m2);
	sh->dx -= k * dx / m1;
	sh->dy -= k * dy / m1;
	en->dx += k * dx / m2;
	en->dy += k * dy / m2;

	shDamage(sh, 100);
	shDamage(en, 100);
	/* be sure ships are far enough before next collision test */
	sh->x += sh->dx * 30;
	sh->y += sh->dy * 30;
	en->x += en->dx * 30;
	en->y += en->dy * 30;
}

void shUpdateShips(float dt) {
	ship_t * sh;
	int l;
	for (sh = head; sh != NULL; sh = sh->next) {
		if (sh->in.direction) {
			sh->r += sh->in.direction * dt * sh->t->maniability;
		}
		if (sh->in.acceleration) {
			sh->dx += sh->t->thrust * dt * cos(sh->r);
			sh->dy += sh->t->thrust * dt * sin(sh->r);
		}
		sh->x += sh->dx * dt;
		sh->y += sh->dy * dt;
		if (sh->in.fire1) {
			for (l = 0; l < sh->t->numlaser; l++) {
				firelaser(sh, &sh->t->laser[l]);
			}
		}
	}
	for (sh = head; sh != NULL; sh = sh->next) {
		ship_t * en;
		float dx,dy,s;
		for (en = sh->next; en != NULL; en = en->next) {
			dx = en->x - sh->x;
			dy = en->y - sh->y;
			s = (en->t->shieldsize + sh->t->shieldsize) / 2.f;
			s = s*s;
			if (dx * dx + dy * dy < s) {
				shCollide(sh,en, dx, dy);
			}
		}
	}


	for (sh = head; sh != NULL; sh = sh->next) {
		if (sh->drawshield > 0)
			sh->drawshield -= dt;
	}
}

ship_t * shFindNearestEnemy(ship_t * self) {
	ship_t * sh = NULL;
	for (sh = head; sh != NULL; sh = sh->next) {
		if (sh == self)
			continue;
		if (sh->team != self->team)
			return sh;
	}
	return NULL;
}

void shDrawShips(void) {
	ship_t * sh;
	for (sh = head; sh != NULL; sh = sh->next) {
		grSetBlend(sh->t->tex);
		grBlitRot(sh->x, sh->y, sh->r, sh->t->size);
		if (sh->drawshield > 0) {
			grSetBlendAdd(sh->t->shieldtex);
			grBlit(sh->x, sh->y, sh->t->shieldsize * M_SQRT1_2, 0);
		}
	}
}

