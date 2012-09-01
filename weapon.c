/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "vec.h"
#include "particle.h"
#include "ship.h"
/* For testing only */
#define NBPART 1000

typedef struct {
	float size;
	unsigned int color;
	traj_t traj;
	short int maxlife;
} bullet_t;

static bullet_t *bul;
static int freeBul = 0;

void weInit(void) {
    bul = malloc(NBPART * sizeof(*bul));
	memset(bul, 0, NBPART * sizeof(*bul));
}

void weLaser(int netid, pos_t *p, float len, float width, float lifetime, unsigned int color, float time) {
	int i;
	i = freeBul;
	bul[i].maxlife = time + lifetime;
	bul[i].traj.base = *p;
	bul[i].size = len;
    bul[i].traj.basetime = time;
	bul[i].color = color;

	paLas(*p, len, color);
	freeBul++;
	if (freeBul >= NBPART)
		freeBul = 0;
}

void weUpdate(float time) {
	int i;
    pos_t p;
    ship_t *tg;
	for (i = 0; i < NBPART; i++) {
		if (time >= bul[i].maxlife)
			continue;
        get_pos(time, &bul[i].traj, &p);
//        tg = shFindShip(&p);
        if (tg)
            shDamage(tg, 10.);
	}
}
