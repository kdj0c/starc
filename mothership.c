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
	pos_t tmp;

	sh->health = sh->t->maxhealth;

	get_pos(time, &ms->traj, &tmp);

	sh->traj.base.p = vmatrix(tmp.p, ms->t->part[ms->t->hangar].p, tmp.r);
	sh->traj.base.r = tmp.r + ms->t->part[ms->t->hangar].r - M_PI_2;
	sh->traj.base.v = vadd(ms->pos.v, vangle(1., sh->traj.base.r));
	sh->traj.base.dr = 0.;
	sh->traj.basetime = time;
}
