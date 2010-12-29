/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <math.h>
#include <string.h>
#include "ship.h"
#include "ai.h"

#define MAX_AIM (5000 * 5000)

enum {
	ai_aim,
	ai_approach,
	ai_synchronize
};

static ai_t * aihead = NULL;

static void addAi(ai_t * ai) {
	if (aihead) {
		ai->next = aihead->next;
		aihead->next = ai;
	} else {
		aihead = ai;
		ai->next = NULL;
	}
}

ai_t * aiCreate(ship_t * sh) {
	ai_t * newai;

	newai = malloc(sizeof(ai_t));
	newai->ship = sh;
	newai->target = NULL;
	newai->state = ai_aim;
	addAi(newai);
	return newai;
}

void aiThink(void) {
	ai_t * ai;
	ship_t * sh;
	ship_t * tg;
	float tx, ty, dx, dy, d, tdx, tdy, s;

	for (ai = aihead; ai != NULL; ai = ai->next) {

		sh = ai->ship;
		tg = ai->target;

		sh->in.acceleration = 0;
		sh->in.direction = 0;
		sh->in.fire1 = 0;

		if (!tg || tg->health <= 0) {
			ai->target = shFindNearestEnemy(sh);
			if (!ai->target)
				return;
			tg = ai->target;
		}

		dx = tg->x - sh->x;
		dy = tg->y - sh->y;

		tx = dx * cos(sh->r) + dy * sin(sh->r);
		ty = dx * sin(sh->r) - dy * cos(sh->r);

		d = dx * dx + dy * dy;

		tdx = tg->dx - sh->dx;
		tdy = tg->dy - sh->dy;

		s = tg->t->shieldsize / 2.;

		switch (ai->state) {
		case ai_aim:
			if (ty < 0)
				sh->in.direction = 1;
			else
				sh->in.direction = -1;

			if (d > MAX_AIM) {
				ai->state = ai_approach;
			} else if ((d < LASER_RANGE * LASER_RANGE) && tx > 0 && ty > -s
					&& ty < s) {
				sh->in.fire1 = 1;
			}
			if (d <= ((dx + tdx) * (dx + tdx) + (dy + tdy) * (dy + tdy)) && ty < s && ty > -s) {
				sh->in.acceleration = 1;
			}
			break;
		case ai_approach:
			if (ty < 0)
				sh->in.direction = 1;
			else
				sh->in.direction = -1;

			if (ty < s && ty > -s)
				sh->in.acceleration = 1;

			if ( d < LASER_RANGE * LASER_RANGE)
				ai->state = ai_aim;

			break;
		case ai_synchronize:
			break;
		}
	}
}
