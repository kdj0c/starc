/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "list.h"
#include "ship.h"
#include "ai.h"
#include "event.h"

#define MAX_AIM 7000
#define VAPP 2.0
#define ADJ_VAPP 0.00004

enum {
	ai_aim,
	ai_approach,
};

LIST_HEAD(aih);

ai_t *aiCreate(ship_t *sh) {
	ai_t *newai;

	newai = malloc(sizeof(ai_t));
	newai->ship = sh;
	newai->target = NULL;
	newai->state = ai_aim;
	list_add(&newai->list, &aih);
	return newai;
}

void aiThinkAll(float time) {
	ai_t *ai;
	shin_t in;

	list_for_each_entry(ai, &aih, list) {
		in.acceleration = 0;
		in.direction = 0;
		in.fire1 = 0;
		aiThink(ai, &in, time);

		if (memcmp(&ai->ship->in, &in, sizeof(in)))
			evPostTrajEv(&in, ai->ship->netid);
	}
}

void aiThink(ai_t *ai, shin_t *in, float time) {
	ship_t *sh;
	ship_t *tg;
	float d, s;
	float ndx, ndy, nr, dd;
	vec_t di, t, td, tmp;
	pos_t tp, mp;

	sh = ai->ship;
	tg = ai->target;

	if (sh->health <= 0)
		return;

	/* need a specific IA for mothership */
	if (sh->t->flag & SH_MOTHERSHIP)
		return;

	if (!tg || tg->health <= 0) {
		ai->target = shFindNearestEnemy(sh);
		if (!ai->target)
			return;
		tg = ai->target;
	} else {
		ship_t *newtg;
		newtg = shFindNearestEnemy(sh);
		if (newtg != tg) {
			if (sqdist(newtg->pos.p, sh->pos.p) < sqdist(tg->pos.p, sh->pos.p) + 2000 * 2000) {
				tg = newtg;
				ai->target = tg;
			}
		}
	}
	// get ship and target position 100ms in the futur
	get_pos(time + 100., &tg->traj, &tp);
	get_pos(time + 100., &sh->traj, &mp);

	di = vsub(tp.p, mp.p);
	t = vmatrix1(di, mp.r);
	d = norm(di);
	td = vsub(tp.v, mp.v);
	tmp = vadd(td, di);

	s = tg->t->shieldsize / 2.;

	switch (ai->state) {
		/*
		 * Just aim at the enemy, and fire if it is within range
		 */
	case ai_aim:
		if (t.y < 0)
			in->direction = 1;
		else
			in->direction = -1;

		if (d > MAX_AIM) {
			ai->state = ai_approach;
		} else if ((d < LASER_RANGE) && t.x > 0 && t.y > -s && t.y < s) {
			in->fire1 = 1;
		}
		if (d <= norm(tmp)
			&& t.y < s && t.y > -s) {
			in->acceleration = 1;
		}
		break;
		/*
		 * try to approach target at a speed of VAPP
		 */
	case ai_approach:
		ndx = tp.v.x + VAPP * (ADJ_VAPP * (d - LASER_RANGE) + 1.)
			* (di.x / d) - mp.v.x;
		ndy = tp.v.y + VAPP * (di.y / d) - mp.v.y;
		dd = ndx * ndx + ndy * ndy;
		nr = atan2(ndy, ndx) - mp.r;
		while (nr > M_PI)
			nr -= 2 * M_PI;
		while (nr < -M_PI)
			nr += 2 * M_PI;

		if (nr > 0)
			in->direction = 1;
		else
			in->direction = -1;

		if (nr < 0.1 && nr > -0.1 && dd > 0.1)
			in->acceleration = 1;

		if (d < 5000.)
			ai->state = ai_aim;
		break;
	}
}
