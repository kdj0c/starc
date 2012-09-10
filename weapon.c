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
#include "ship.h"
#include "graphic.h"
/* For testing only */
#define NBPROJ 1000

typedef struct {
	traj_t traj;
	float maxlife;
	unsigned int color;
	int netid;
} bullet_t;

static bullet_t *bul;
static int freeBul = 0;
#ifndef DEDICATED
static unsigned int wetex;
#endif

void weInit(void) {
    bul = malloc(NBPROJ * sizeof(*bul));
	memset(bul, 0, NBPROJ * sizeof(*bul));
#ifndef DEDICATED
    wetex = grLoadTexture("img/particle.png");
#endif
}

void weMissile(int netid, pos_t *p, unsigned int color, float time) {
	int i;
	i = freeBul;
	bul[i].maxlife = time + 5000.;
	p->v = vadd(p->v, vangle(5., p->r));
	bul[i].traj.base = *p;
    bul[i].traj.basetime = time;
    bul[i].traj.type = t_linear;
    bul[i].color = color | 0xFF;
    bul[i].netid = netid;
	freeBul++;
	if (freeBul >= NBPROJ)
		freeBul = 0;
}

void weUpdate(float time) {
    int i;
    pos_t p;
	for (i = 0; i < NBPROJ; i++) {
		if (time >= bul[i].maxlife)
			continue;
        get_pos(time, &bul[i].traj, &p);
        if (shDetectHit(bul[i].netid, &p, 150., time)) {
            bul[i].maxlife = time;
        }
	}
}

#ifndef DEDICATED
void weDraw(float time) {
	int i;
    pos_t p;

    grSetBlendAdd(wetex);
	for (i = 0; i < NBPROJ; i++) {
		if (time >= bul[i].maxlife)
			continue;

        get_pos(time, &bul[i].traj, &p);
        grSetColor(bul[i].color);
        grBlitSquare(p.p.x, p.p.y, 150.);
	}
}
#endif
