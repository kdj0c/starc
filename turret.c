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
		new[i].health = t->t->maxhealth;
	}
}

void tuDamage(turret_t *tu, float dg, float time) {
    tu->health -= dg;
    tu->lastdamage = time;
}

void tufirelaser(ship_t * sh, turret_t * tu, laser_t * las, float time) {
    pos_t p;

    p.p = vmatrix(tu->p, las->p, tu->r);
    p.r = tu->r + las->r;
    p.v = sh->pos.v;
    evPostLaser(sh->netid, &p, las->color, 200., LASER_RANGE, 20., time);
}

float tuGetAim(turret_t *tu, float m, float time) {
    return tu->baseaim + m * (float) tu->dir * (time - tu->basetime);
}

void tuSetMove(int netid, signed char *dir, float time) {
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
    pos_t tp;
    turret_t *res = NULL;
    vec_t d, d1;
    float s;

	for (i = 0; i < sh->t->numturret; i++) {
		t = &sh->t->turret[i];
		tu = &sh->turret[i];
		tp.p = vmatrix(ms->p, t->p, ms->r);

		if (tu->health <= 0)
			continue;

        d = vsub(tp.p, p->p);
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

void tuUpdate(ship_t *sh, float time) {
	turret_t *tu;
	turretpos_t *t;
	int i,l;
	pos_t mp;
    signed char dir[MAX_TURRET];

	get_pos(time, &sh->traj, &mp);

	for (i = 0; i < sh->t->numturret; i++) {
		t = &sh->t->turret[i];
		tu = &sh->turret[i];

		if (time - tu->lastthink > 500.) {
			tu->target = shFindNearestEnemy(sh);
			tu->lastthink = time;
		}

		tu->p = vmatrix(mp.p, t->p, mp.r);
		if (tu->target) {
			vec_t d, d1;
			pos_t tp;
			float a;

			get_pos(time + 100., &tu->target->traj, &tp);
			tu->r = tuGetAim(tu, t->t->maniability, time + 100.);
			d = vsub(tp.p, tu->p);
			d1 = vmatrix1(d, tu->r);
			a = atan2(d1.y, d1.x);
			if (a > .05)
				dir[i] = -1;
			else if (a < .05)
				dir[i] = 1;
            else
                dir[i] = 0;

			if (norm(d) < LASER_RANGE && ((time - tu->lastfire) > 200.) &&
			a < .5 && a > -.5) {
				for (l = 0; l < t->t->numlaser; l++) {
					tufirelaser(sh, tu, &t->t->laser[l], time);
				}
				tu->lastfire = time;
			}
		} else {
		    dir[i] = 0;
		}
	}
    evPostTurret(sh->netid, dir, time);
}
#ifndef DEDICATED
void tuDraw(ship_t * sh, float time) {
	turret_t *tu;
	turretpos_t *t;
	int i;

	for (i = 0; i < sh->t->numturret; i++) {
		t = &sh->t->turret[i];
		tu = &sh->turret[i];
		tu->p = vmatrix(sh->pos.p, t->p, sh->pos.r);
		tu->r = tuGetAim(tu, t->t->maniability, time);
        grSetBlend(sh->t->turret[i].t->tex);
		grBlitRot(tu->p.x, tu->p.y, tu->r, 700.);
        if (time - tu->lastdamage < 500.) {
			grSetBlendAdd(t->t->shieldtex);
			grBlit(tu->p.x, tu->p.y, t->t->shieldsize * M_SQRT1_2, 0.);
		}

	}
}
#endif
