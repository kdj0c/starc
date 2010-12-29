/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <math.h>
#include "ship.h"
#include "ai.h"

ai_t * aiCreate(ship_t * sh, ship_t * target) {
	ai_t * newai;

	newai = malloc(sizeof(ai_t));
	newai->ship = sh;
	newai->target = target;
	return newai;
}

void aiThink(ai_t *ai) {
	ship_t * sh;
	ship_t * tg;
	float tx, ty, dx, dy;

	sh = ai->ship;
	tg = ai->target;

	dx = sh->x - tg->x;
	dy = sh->y - tg->y;

	tx = dx * cos(sh->r) + dy * sin(sh->r);
	ty = dx * sin(sh->r) - dy * cos(sh->r);

	if (ty > 0)
		sh->in.direction = 1;
	else
		sh->in.direction = -1;


}
