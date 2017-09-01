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
		new[i].p = vmatrix(sh->pos.p, t->p, sh->pos.r);
		new[i].r = sh->pos.r;
		new[i].health = t->t->maxhealth;
	}
}

void tuDamage(turret_t *tu, float dg, float time) {
	tu->health -= dg;
	tu->lastdamage = time;
}

void tufirelaser(ship_t *sh, turret_t *tu, turrettype_t *t, int l, float time) {
	pos_t p;
	weapon_t *las = &t->laser[l];

	p.p = vmatrix(tu->p, las->p, tu->r);
	p.r = tu->r + las->r;
	p.v = sh->pos.v;
	evPostFire(sh->netid, &p, l, time);
	tu->lastfire[l] = time;
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

turret_t *tuCheckTurret(ship_t *sh, pos_t *p, pos_t *ms, float len, float *min) {
	turret_t *tu;
	turretpos_t *t;
	int i;
	vec_t tp;
	turret_t *res = NULL;
	vec_t d, d1;
	float s;

	for (i = 0; i < sh->t->numturret; i++) {
		t = &sh->t->turret[i];
		tu = &sh->turret[i];
		tp = vmatrix(ms->p, t->p, ms->r);

		if (tu->health <= 0)
			continue;

		d = vsub(tp, p->p);
		s = t->t->shieldsize / 2.f;

		if (norm(d) > LASER_RANGE + s)
			continue;

		d1 = vmatrix1(d, p->r);
		if (d1.x > 0 && d1.x < LASER_RANGE + s && d1.y > -s && d1.y < s) {
			len = d1.x - sqrt(s * s - d1.y * d1.y);
			if (len < *min) {
				*min = len;
				res = tu;
			}
		}
	}
	return res;
}

int tuCheckTurretProj(ship_t *sh, pos_t *p, pos_t *ms, float len) {
	turret_t *tu;
	turretpos_t *t;
	int i;
	vec_t tp;
	vec_t d;
	float s;

	for (i = 0; i < sh->t->numturret; i++) {
		t = &sh->t->turret[i];
		tu = &sh->turret[i];
		tp = vmatrix(ms->p, t->p, ms->r);

		if (tu->health <= 0)
			continue;

		d = vsub(tp, p->p);
		s = t->t->shieldsize / 2.f;

		if (norm(d) > s + len)
			continue;
		return i;
	}
	return -1;
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

		tu->p = vmatrix(mp.p, t->p, mp.r);
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
						tufirelaser(sh, tu, t->t, l, time);
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
		tu->p = vmatrix(sh->pos.p, t->p, sh->pos.r);
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
		tu->p = vmatrix(sh->pos.p, t->p, sh->pos.r);
		tu->r = tuGetAim(tu, t->t->maniability, time);
		if (time - tu->lastdamage < 500.) {
			grBatchAdd(tu->p, t->t->shieldsize * M_SQRT1_2, 0., &t->t->shieldtex, t->t->shieldcolor);
		}
	}
}
#endif
