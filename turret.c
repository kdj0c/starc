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
		new[i].p = vmatrix(sh->pos.p, t->p, sh->pos.r);
		new[i].r = sh->pos.r;
	}
}

void tufirelaser(ship_t * sh, turret_t * tu, laser_t * las, float dt) {
    pos_t p;

    p.p = vmatrix(tu->p, las->p, tu->r);
    p.r = tu->r + las->r;
    p.v = sh->pos.v;
//	shFireLaser(p, sh, las, dt);
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

		tu->p = vmatrix(sh->pos.p, t->p, sh->pos.r);
		if (tu->target) {
			float dx, dy, ty;
			dx = tu->target->pos.p.x - tu->p.x;
			dy = tu->target->pos.p.y - tu->p.y;
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
		grBlitRot(tu->p.x, tu->p.y, tu->r, 700.);
	}
}
#endif
