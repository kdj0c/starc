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

ai_t * aiCreate(ship_t * sh) {
	ai_t * newai;

	newai = malloc(sizeof(ai_t));
	newai->ship = sh;
	newai->target = NULL;
	newai->state = ai_aim;
	list_add(&newai->list, &aih);
	return newai;
}

void aiThink(void) {
	ai_t * ai;
	ship_t * sh;
	ship_t * tg;
	float tx, ty, dx, dy, d, tdx, tdy, s;
	float ndx, ndy, nr, dd;
	shin_t in;

	list_for_each_entry(ai, &aih, list) {
		sh = ai->ship;
		tg = ai->target;

		in.acceleration = 0;
		in.direction = 0;
		in.fire1 = 0;

		if(sh->health <= 0)
			continue;

		/* need a specific IA for mothership */
		if(sh->t->flag & SH_MOTHERSHIP)
			continue;

		if (!tg || tg->health <= 0) {
			ai->target = shFindNearestEnemy(sh);
			if (!ai->target)
				continue;
			tg = ai->target;
		} else {
			ship_t * newtg;
			newtg = shFindNearestEnemy(sh);
			if (newtg != tg) {
				float dx2, dy2;
				dx = tg->x - sh->x;
				dy = tg->y - sh->y;
				dx2 = newtg->x - sh->x;
				dy2 = newtg->y - sh->y;
				if (dx2 * dx2 + dy2 * dy2 < dx * dx + dy * dy + 2000 * 2000) {
					tg = newtg;
					ai->target = tg;
				}
			}
		}

		dx = tg->x - sh->x;
		dy = tg->y - sh->y;

		tx = dx * cos(sh->r) + dy * sin(sh->r);
		ty = dx * sin(sh->r) - dy * cos(sh->r);

		d = sqrt(dx * dx + dy * dy);

		tdx = tg->dx - sh->dx;
		tdy = tg->dy - sh->dy;

		s = tg->t->shieldsize / 2.;

		switch (ai->state) {
		/*
		 * Just aim at the enemy, and fire if it is within range
		 */
		case ai_aim:
			if (ty < 0)
				in.direction = 1;
			else
				in.direction = -1;

			if (d > MAX_AIM) {
				ai->state = ai_approach;
			} else if ((d < LASER_RANGE) && tx > 0 && ty > -s && ty < s) {
				in.fire1 = 1;
			}
			if (d <= sqrt((dx + tdx) * (dx + tdx) + (dy + tdy) * (dy + tdy))
					&& ty < s && ty > -s) {
				in.acceleration = 1;
			}
			break;
		/*
		 * try to approach target at a speed of VAPP
		 */
		case ai_approach:
			ndx = tg->dx + VAPP * (ADJ_VAPP * (d - LASER_RANGE) + 1.)
					* (dx / d) - sh->dx;
			ndy = tg->dy + VAPP * (dy / d) - sh->dy;
			dd = ndx * ndx + ndy * ndy;
			nr = atan2(ndy, ndx) - sh->r;
			while (nr > M_PI)
				nr -= 2 * M_PI;
			while (nr < -M_PI)
				nr += 2 * M_PI;

			if (nr > 0)
				in.direction = 1;
			else
				in.direction = -1;

			if (nr < 0.1 && nr > -0.1 && dd > 0.1)
				in.acceleration = 1;

			if (d < LASER_RANGE)
				ai->state = ai_aim;
			break;
		}
	evPostTrajEv(&in, sh->netid);
	}
}
