/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "ship.h"
#include "graphic.h"
#include "particle.h"

#ifndef DEDICATED

int tuTex=0;
void tuLoadTurret(void) {
   tuTex = grLoadTexture("img/tu.png");
}
#endif


void tuAddTurret(ship_t * sh) {
	turret_t *new;
	turretpos_t *t;
	int i;

	new = malloc(sizeof(*new) * sh->t->numturret);
	memset(new,0,sizeof(*new) * sh->t->numturret);
	sh->turret = new;

	for (i = 0; i < sh->t->numturret; i++) {
		t = &sh->t->turret[i];
		new[i].x = sh->x + t->x * cos(sh->r) + t->y * sin(sh->r);
		new[i].y = sh->y + t->x * sin(sh->r) - t->y * cos(sh->r);
		new[i].r = sh->r;
	}
}

void tufirelaser(ship_t * sh, turret_t * tu, laser_t * las, float dt) {
	float x, y, r;
	x = tu->x + las->x * cos(tu->r) + las->y * sin(tu->r);
	y = tu->y + las->x * sin(tu->r) - las->y * cos(tu->r);
	r = tu->r + las->r;

	shFireLaser(x, y, r, sh->dx, sh->dy, las, dt, sh);
}

void tuUpdate(ship_t *sh, float dt) {
	turret_t *tu;
	turretpos_t *t;
	int i,l;

	for (i = 0; i < sh->t->numturret; i++) {
		t = &sh->t->turret[i];
		tu = &sh->turret[i];

		tu->last_think += dt;

		if (tu->last_think > 500.) {
			tu->target = shFindNearestEnemy(sh);
			tu->last_think = 0;
		}

		tu->x = sh->x + t->x * cos(sh->r) + t->y * sin(sh->r);
		tu->y = sh->y + t->x * sin(sh->r) - t->y * cos(sh->r);
		if (tu->target) {
			float dx, dy, ty;
			dx = tu->target->x - tu->x;
			dy = tu->target->y - tu->y;
			ty = dx * sin(tu->r) - dy * cos(tu->r);
			if (ty > 10.)
				tu->r -= t->t->maniability * dt;
			else if (ty < 10.)
				tu->r += t->t->maniability * dt;

			if (dx * dx + dy * dy < LASER_RANGE * LASER_RANGE) {
				for (l = 0; l < t->t->numlaser; l++) {
					tufirelaser(sh, tu, &t->t->laser[l], dt);
				}
			}
		}
	}
}
#ifndef DEDICATED
void tuDraw(ship_t * sh) {
	turret_t *tu;
	int i;

   grSetBlend(tuTex);
	for (i = 0; i < sh->t->numturret; i++) {
		tu = &sh->turret[i];
		grBlitRot(tu->x, tu->y, tu->r, 700.);
	}
}
#endif
