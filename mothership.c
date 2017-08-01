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

void msRespawn(ship_t *sh, ship_t *ms, float time) {
	hangar_t *hg;
	pos_t tmp;

	sh->health = sh->t->maxhealth;
	hg = &ms->t->hangar;

	get_pos(time, &ms->traj, &tmp);

	sh->traj.base.p = vmatrix(tmp.p, hg->p, tmp.r);
//  sh->traj.base.p.x = tmp->p.x + hg->x * cos(tmp->r) + hg->y * sin(tmp->r);
//  sh->traj.base.p.y = tmp->p.y + hg->x * sin(tmp->r) - hg->y * cos(tmp->r);
	sh->traj.base.r = tmp.r - hg->r;
	sh->traj.base.v = vadd(ms->pos.v, vangle(1., sh->traj.base.r));
	sh->traj.basetime = time;
}
