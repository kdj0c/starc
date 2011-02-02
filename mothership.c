/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <math.h>
#include "ship.h"

void msInitMotherShip() {

}

void msRespawn(ship_t * sh) {
	ship_t *ms;
	hangar_t *hg;
	sh->health = sh->t->maxhealth;
	ms = shFindMotherShip(sh->team);
	if (!ms)
		return;
	hg = &ms->t->hangar;

	sh->x = ms->x + hg->x * cos(ms->r) + hg->y * sin(ms->r);
	sh->y = ms->y + hg->x * sin(ms->r) - hg->y * cos(ms->r);
	sh->r = ms->r - hg->r;
	sh->dx = 1. * cos(sh->r) + ms->dx;
	sh->dy = 1. * sin(sh->r) + ms->dy;
	sh->drawshield = 0;
}
