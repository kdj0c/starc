/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdlib.h>
#include <math.h>

#include "graphic.h"
/* For testing only */
#define NBPART 100000

typedef struct {
	float x;
	float y;
	float dx;
	float dy;
	float size;
	float c;
	unsigned int color;
	int maxlife;
	int life;
} particle_t;

static particle_t * parts;
static unsigned int texture;
static int freePart = 0;

void paInit(void) {
	parts = malloc(NBPART * sizeof(*parts));
	texture = grLoadTexture("img/particle.png");
}

void paExplosion(float x, float y, float v, int number) {
	int i;

	if( freePart + number >= NBPART)
		freePart = 0;

	for (i = freePart; i < freePart + number; i++) {
		float len,angle,rx,ry;

		len = (float) ((rand() % 1000) - 500) / 500.f;
		angle = (float) (rand() % 1000) * M_PI / 500.f;
		rx = len * cos(angle);
		ry = len * sin(angle);
		parts[i].maxlife = rand() % 1000;
		parts[i].x = x + rx * 10.0;
		parts[i].y = y + ry * 10.0;
		parts[i].dx = rx * v;
		parts[i].dy = ry * v;
		parts[i].life = parts[i].maxlife;
		parts[i].color = 0xFFA0A000;
		parts[i].size = rand() % 100 + 50;
	}
	freePart += number;
}

void paBurst(float x, float y, float dx, float dy, float r, unsigned int color) {
	int i;
	i = freePart;
	parts[i].maxlife = rand() % 1000 + 1000;
	parts[i].x = x;
	parts[i].y = y;
	parts[i].dx = dx - 0.8 * cos(r) + 0.2 * ((rand() % 1000 - 500) / 500.f);
	parts[i].dy = dy - 0.8 * sin(r) + 0.2 * ((rand() % 1000 - 500) / 500.f);
	parts[i].life = parts[i].maxlife;
	parts[i].size = rand() % 100 + 50;
	parts[i].color = color;
	freePart++;
	if (freePart >= NBPART)
		freePart = 0;
}

void paLaser(float x, float y, float dx, float dy, unsigned int color) {
	int i;
	i = freePart;
	parts[i].maxlife = rand() % 500 + 50;
	parts[i].x = x;
	parts[i].y = y;
	parts[i].dx = dx;
	parts[i].dy = dy;
	parts[i].life = parts[i].maxlife;
	parts[i].size = rand() % 100 + 50;
	parts[i].color = color;
	freePart++;
	if (freePart >= NBPART)
		freePart = 0;
}

void paUpdate(float dt) {
	int i;
	grSetBlendAdd(texture);
	for (i = 0; i < NBPART; i++) {
		if (parts[i].life <= 0)
			continue;
		parts[i].x += parts[i].dx * dt;
		parts[i].y += parts[i].dy * dt;
		parts[i].life -= dt;
		parts[i].c = (float) parts[i].life / (float) parts[i].maxlife;
		parts[i].color &= ~0xFF;
		parts[i].color |= (int) (parts[i].c * 255);
		if (parts[i].life > 0) {
			grSetColor(parts[i].color);
			grBlitSquare(parts[i].x, parts[i].y, parts[i].size);
		}
	}
}
