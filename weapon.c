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
#include "particle.h"
#include "event.h"
#include "config.h"
/* For testing only */
#define NBPROJ 1000

typedef struct {
	traj_t traj;
	float maxlife;
	unsigned int color;
	int netid;
	weapontype_t *type;
} bullet_t;

static bullet_t *bul;
static int freeBul = 0;

void weInit(void) {
	bul = malloc(NBPROJ * sizeof(*bul));
	memset(bul, 0, NBPROJ * sizeof(*bul));
}

int weGetFree(void) {
	int i;

	i = freeBul;
	freeBul++;
	if (freeBul >= NBPROJ)
		freeBul = 0;
	return i;
}

void weFire(int netid, pos_t *p, weapontype_t *wt, float time) {
	int i;
	i = weGetFree();
	bul[i].maxlife = time + wt->lifetime;
	p->v = vadd(p->v, vangle(wt->speed, p->r));
	bul[i].traj.base = *p;
	bul[i].traj.basetime = time;
	if (wt->type == WE_LASER)
		bul[i].traj.type = t_linear;
	else {
		bul[i].traj.type = t_linear_acc;
		bul[i].traj.thrust = 0.003;
	}
	bul[i].color = wt->color | 0xFF;
	bul[i].netid = netid;
	bul[i].type = wt;
}

void weUpdate(float time) {
	int i;
	pos_t p;

	for (i = 0; i < NBPROJ; i++) {
		if (time >= bul[i].maxlife)
			continue;
		get_pos(time, &bul[i].traj, &p);
		if (shDetectHit(bul[i].netid, &p, 150., i, time))
			bul[i].maxlife = time;
	}
}

float weHit(int id, pos_t *p, float time) {
	if (bul[id].type->type == WE_LASER)
		paLaserHit(p->p, p->v, bul[id].color, time);
	bul[id].maxlife = 0.;
	return (float) bul[id].type->damage;
}

#ifndef DEDICATED
void weDraw(float time) {
	int i;
	pos_t p;

	grSetBlend();
	for (i = 0; i < NBPROJ; i++) {
		if (time >= bul[i].maxlife)
			continue;

		get_pos(time, &bul[i].traj, &p);
		grBatchAddRot(p.p, p.r, &bul[i].type->texture, 0xFFFFFFFF);
	}
	grBatchDraw();
}
#endif
