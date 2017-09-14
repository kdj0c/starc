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
#include "event.h"
#include "weapon.h"

void tuAddTurret(ship_t *sh) {
	turret_t *new;
	turretpos_t *t;
	int i;

	new = malloc(sizeof(*new) * sh->t->numturret);
	memset(new, 0, sizeof(*new) * sh->t->numturret);
	sh->turret = new;

	for (i = 0; i < sh->t->numturret; i++) {
		t = &sh->t->turret[i];
		new[i].p = vmatrix(sh->pos.p, sh->t->part[t->i].p, sh->pos.r);
		new[i].r = sh->pos.r + sh->t->part[t->i].r;
		new[i].health = t->t->maxhealth;
	}
}

void tuDamage(turret_t *tu, float dg, float time) {
	tu->health -= dg;
	tu->lastdamage = time;
}

void tufirelaser(ship_t *sh, turret_t *tu, turrettype_t *t, int i, int l, float time) {
	pos_t p;
	weapon_t *las = &t->laser[l];

	p.p = vmatrix(tu->p, las->p, tu->r);
	p.r = tu->r + las->r;
	p.v = sh->pos.v;
	evPostFire(sh->netid, &p, MAX_WEAPON + i * MAX_TURRET + l, sh->weId, time);
	tu->lastfire[l] = time;
	sh->weId++;
	if (sh->weId >= MAX_WEID)
		sh->weId = 0;
}

float tuGetAim(turret_t *tu, float m, float time) {
	return tu->baseaim + m * (float) tu->dir * (time - tu->basetime);
}

void tuSetMove(int netid, float *dir, float time) {
	turret_t *tu;
	turretpos_t *t;
	ship_t *sh;
	int i;

	sh = shGetByID(netid);

	for (i = 0; i < sh->t->numturret; i++) {
		t = &sh->t->turret[i];
		tu = &sh->turret[i];
		if (dir[i] != tu->dir) {
			tu->baseaim = tuGetAim(tu, t->t->maniability, time);
			tu->basetime = time;
			tu->dir = dir[i];
		}
	}
}

void tuUpdate(ship_t *sh, float time) {
	turret_t *tu;
	turretpos_t *t;
	int i, l;
	pos_t mp;
	float dir[MAX_TURRET];

	get_pos(time, &sh->traj, &mp);

	for (i = 0; i < sh->t->numturret; i++) {
		t = &sh->t->turret[i];
		tu = &sh->turret[i];

		if (tu->health <= 0) {
			dir[i] = 0;
			continue;
		}

		if (time - tu->lastthink > 500.) {
			tu->target = shFindNearestEnemy(sh);
			tu->lastthink = time;
		}

		tu->p = vmatrix(mp.p, sh->t->part[t->i].p, mp.r);
		if (tu->target) {
			vec_t d, d1;
			pos_t tp;
			float a;

			get_pos(time, &tu->target->traj, &tp);
			tu->r = tuGetAim(tu, t->t->maniability, time);
			d = vsub(tp.p, tu->p);
			d1 = vmatrix1(d, tu->r);
			a = -atan2f(d1.y, d1.x);

			if (a < .03 && a > -.03 && norm(d) < LASER_RANGE) {
				for (l = 0; l < t->t->numweapon; l++) {
					if (time - tu->lastfire[l] > t->t->laser[l].wt->firerate) {
						tufirelaser(sh, tu, t->t, i, l, time);
						tu->lastfire[l] = time;
					}
				}
			}

			a /= t->t->maniability * 100.;
			if (a > 1.)
				a = 1.;
			if (a < -1.)
				a = -1.;

			dir[i] = a;
		} else {
			dir[i] = 0;
		}
	}
	evPostTurret(sh->netid, dir, time);
}

#ifndef DEDICATED
void tuDraw(ship_t *sh, float time) {
	turret_t *tu;
	turretpos_t *t;
	int i;

	for (i = 0; i < sh->t->numturret; i++) {
		t = &sh->t->turret[i];
		tu = &sh->turret[i];
		tu->p = vmatrix(sh->pos.p, sh->t->part[t->i].p, sh->pos.r);
		tu->r = tuGetAim(tu, t->t->maniability, time);
		grBatchAddRot(tu->p, tu->r, &t->t->tex, 0xFFFFFFFF);
	}
}

void tuDrawShields(ship_t *sh, float time) {
	turret_t *tu;
	turretpos_t *t;
	int i;

	for (i = 0; i < sh->t->numturret; i++) {
		t = &sh->t->turret[i];
		tu = &sh->turret[i];
		tu->p = vmatrix(sh->pos.p, sh->t->part[t->i].p, sh->pos.r);
		tu->r = tuGetAim(tu, t->t->maniability, time);
		if (time - tu->lastdamage < 500.) {
			grBatchAdd(tu->p, t->t->shieldsize * M_SQRT1_2, 0., &t->t->shieldtex, t->t->shieldcolor);
		}
	}
}
#endif
