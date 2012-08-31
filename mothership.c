/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <math.h>
#include <GL/glut.h>
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

	sh->traj.base.p.x = ms->pos.p.x + hg->x * cos(ms->pos.r) + hg->y * sin(ms->pos.r);
	sh->traj.base.p.y = ms->pos.p.y + hg->x * sin(ms->pos.r) - hg->y * cos(ms->pos.r);
	sh->traj.base.r = ms->pos.r - hg->r;
	sh->traj.base.v = vadd(ms->pos.v, vangle(1., sh->traj.base.r));
	sh->traj.basetime = glutGet(GLUT_ELAPSED_TIME);
	//shNewTraj(&sh->in, sh->netid, time);
	sh->drawshield = 0;
}
