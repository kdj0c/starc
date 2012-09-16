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
/* For testing only */
#define NBPART 100000
#define PA_LASER 0x1


typedef struct {
	float size;
	unsigned int color;
	traj_t traj;
	short int maxlife;
	unsigned int flag;
} particle_t;

static particle_t * parts;
static unsigned int texture;
static unsigned int lastex;
static int freePart = 0;

void paInit(void) {
	parts = malloc(NBPART * sizeof(*parts));
	memset(parts, 0, NBPART * sizeof(*parts));
	texture = grLoadTexture("img/particle.png");
	lastex = grLoadTexture("img/laser.png");
}

void paExplosion(vec_t p, vec_t v, float s, int number, unsigned int color, float time) {
	int i;

	if( freePart + number >= NBPART)
		freePart = 0;

	for (i = freePart; i < freePart + number; i++) {
		float len,angle;
		vec_t v1;
		len = (float) ((rand() % 1000) - 500) / 500.f;
		angle = (float) (rand() % 1000) * M_PI / 500.f;

		v1 = vangle(len, angle);
		parts[i].traj.base.p = vadd(p, vmul(v1, 50.0));
		parts[i].traj.base.v = vadd(v, vmul(v1, s));
		parts[i].traj.basetime = time;
		parts[i].traj.type = t_linear;

		parts[i].maxlife = rand() % 1000 + 500;
		parts[i].color = color;
		parts[i].size = rand() % 200 + 50;
		parts[i].flag = 0;
	}
	freePart += number;
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

	i = freePart;
	parts[i].maxlife = rand() % 1000 + 1000 / size;
	parts[i].size = (rand() % 100 + 50) * size;
	parts[i].color = color;
	parts[i].flag = 0;
	freePart++;
	if (freePart >= NBPART)
		freePart = 0;
}

void paLaser(vec_t p, vec_t v, unsigned int color, float time) {
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

void paLas(pos_t p, float len, unsigned int color) {
	int i;
	i = freePart;
	parts[i].maxlife = rand() % 50 + 150;
//	parts[i].p = p;
	parts[i].size = len;
//	parts[i].life = parts[i].maxlife;
	parts[i].color = color;
	parts[i].flag = PA_LASER;
	freePart++;
	if (freePart >= NBPART)
		freePart = 0;
}

void paLas2(pos_t *p, float len, float width, float lifetime, unsigned int color, float time) {
	int i;
	i = freePart;
	parts[i].maxlife = time + lifetime;
	parts[i].traj.base = *p;
	parts[i].size = len;

	parts[i].maxlife = rand() % 30 + 30;
//	parts[i].p = *p;
	parts[i].size = len;
//	parts[i].life = parts[i].maxlife;
	parts[i].color = color;
	parts[i].flag = PA_LASER;
	freePart++;
	if (freePart >= NBPART)
		freePart = 0;
}

void paDraw(float time) {
	int i;
	float c;
	pos_t p;
	grSetBlendAdd(texture);
	for (i = 0; i < NBPART; i++) {
		if (parts[i].traj.basetime + parts[i].maxlife <= time)
			continue;
		c = 1. - (time - parts[i].traj.basetime) / ((float) parts[i].maxlife);
        parts[i].color &= ~0xFF;
		parts[i].color |= (int) (c * 255);
		grSetColor(parts[i].color);

		get_pos(time, &parts[i].traj, &p);
		if (parts[i].flag == PA_LASER) {
			grSetBlendAdd(lastex);
			grBlitLaser(p.p.x, p.p.y, parts[i].size,
					p.r, 40.);
			grSetBlendAdd(texture);
		} else {
			grBlitSquare(p.p.x, p.p.y, parts[i].size);
		}
	}
}
