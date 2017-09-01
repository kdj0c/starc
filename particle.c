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
#include "graphic.h"
#include "config.h"
/* For testing only */
#define NBPART 2000
#define PA_EXP 0x1

typedef struct {
	float size;
	unsigned int color;
	traj_t traj;
	short int maxlife;
	unsigned int flag;
	unsigned int anim;
} particle_t;

static particle_t *parts;
static texc_t paTex;
static texc_t exTex[4][64];
static int freePart = 0;

void paInit(void) {
	int e, i, j;
	parts = malloc(NBPART * sizeof(*parts));
	memset(parts, 0, NBPART * sizeof(*parts));
	cfGetTexture("particle", &paTex);
	for (e = 0; e < 4; e++) {
		for (i = 0; i < 8; i++) {
			for (j = 0; j < 8; j++) {
				texc_t *t = &exTex[e][i + j * 8];
				t->index = e + 1;
				t->w = 4096. / 4.;
				t->h = 4096. / 4.;
				t->texc[0] = 1. / 8. * i;
				t->texc[1] = 1. / 8. * j;
				t->texc[2] = 1. / 8. * (i + 1);
				t->texc[3] = 1. / 8. * j;
				t->texc[4] = 1. / 8. * (i + 1);
				t->texc[5] = 1. / 8. * (j + 1);
				t->texc[6] = 1. / 8. * i;
				t->texc[7] = 1. / 8. * (j + 1);
			}
		}
	}
}

void paExplosion(vec_t p, vec_t v, float s, int number, unsigned int color, float time) {
	int i;

	i = freePart;

	parts[i].traj.base.p = p;
	parts[i].traj.base.r = ((rand() % 1000) - 500.) * M_PI / 500.;
	parts[i].traj.base.v = v;
	parts[i].traj.basetime = time;
	parts[i].traj.type = t_linear;
	parts[i].maxlife = 1500;
	parts[i].size = s;
	parts[i].flag = PA_EXP;
	parts[i].anim = rand() % 4;

	freePart++;
	if (freePart >= NBPART)
		freePart = 0;
}

void paBurst(pos_t *p, float size, unsigned int color, float time) {
	int i;
	vec_t t;

	i = freePart;

	t.x = 0.2 * ((rand() % 1000 - 500) / 500.f);
	t.y = 0.2 * ((rand() % 1000 - 500) / 500.f);

	parts[i].traj.base.p = p->p;
	parts[i].traj.base.v = vadd(vsub(p->v, vangle(size * 0.8f, p->r)), t);
	parts[i].traj.basetime = time;
	parts[i].traj.type = t_linear;
	parts[i].maxlife = rand() % 1000 + 1000 / size;
	parts[i].size = (rand() % 100 + 50) * size;
	parts[i].color = color;
	parts[i].flag = 0;
	freePart++;
	if (freePart >= NBPART)
		freePart = 0;
}

void paLaserHit(vec_t p, vec_t v, unsigned int color, float time) {
	int i;
	i = freePart;
	parts[i].maxlife = rand() % 800 + 200;
	parts[i].traj.base.p = p;
	parts[i].traj.base.v = v;
	parts[i].traj.basetime = time;
	parts[i].traj.type = t_linear;

	parts[i].size = rand() % 100 + 100;
	parts[i].color = color;
	parts[i].flag = 0;
	freePart++;
	if (freePart >= NBPART)
		freePart = 0;
}

void paDraw(float time) {
	int i;
	float c;
	pos_t p;
	int count;

	count = 0;

	grSetBlendAdd();
	for (i = 0; i < NBPART; i++) {
		if (parts[i].traj.basetime + parts[i].maxlife <= time)
			continue;
		if (parts[i].flag == PA_EXP)
			continue;

		c = 1. - (time - parts[i].traj.basetime) / ((float) parts[i].maxlife);
		parts[i].color &= ~0xFF000000;
		parts[i].color |= ((int) (c * 255)) << 24;
		get_pos(time, &parts[i].traj, &p);

		grBatchAdd(p.p, parts[i].size, 0.f, &paTex, parts[i].color, count);
		count++;
	}
	grBatchDraw(count);
}

void paDrawExplosion(float time) {
	int i;
	pos_t p;
	int count;
	int index;

	count = 0;

	for (i = 0; i < NBPART; i++) {
		if (parts[i].traj.basetime + parts[i].maxlife <= time)
			continue;

		if (parts[i].flag != PA_EXP)
			continue;

		get_pos(time, &parts[i].traj, &p);
		index = (int) (((time - parts[i].traj.basetime) / ((float) parts[i].maxlife)) * 64);
		grBlitRot(p.p, p.r, &exTex[parts[i].anim][index]);
		count++;
	}
}
