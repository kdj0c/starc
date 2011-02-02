/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <math.h>

#include "ship.h"
#include "graphic.h"

void tuUpdate(float dt) {

}

void tuDraw(ship_t * sh) {
	float x, y;
	turret_t * t;
	int i;

	for (i = 0; i < sh->t->numturret; i++) {
		t = &sh->t->turret[i];
		x = sh->x + t->x * cos(sh->r) + t->y * sin(sh->r);
		y = sh->y + t->x * sin(sh->r) - t->y * cos(sh->r);

		grBlitRot(x, y, sh->r, 1000.);
	}
}


