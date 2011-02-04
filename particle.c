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

#include "graphic.h"
/* For testing only */
#define NBPART 100000
#define PA_LASER 0x1


typedef struct {
	float x;
	float y;
	float dx;
	float dy;
	float size;
	float r;
	unsigned int color;
	short int maxlife;
	short int life;
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

void paExplosion(float x, float y, float dx, float dy, float v, int number, unsigned int color) {
	int i;

	if( freePart + number >= NBPART)
		freePart = 0;

	for (i = freePart; i < freePart + number; i++) {
		float len,angle,rx,ry;

		len = (float) ((rand() % 1000) - 500) / 500.f;
		angle = (float) (rand() % 1000) * M_PI / 500.f;
		rx = len * cos(angle);
		ry = len * sin(angle);
		parts[i].maxlife = rand() % 1000 + 500;
		parts[i].x = x + rx * 50.0;
		parts[i].y = y + ry * 50.0;
		parts[i].dx = dx + rx * v;
		parts[i].dy = dy + ry * v;
		parts[i].life = parts[i].maxlife;
		parts[i].color = color;
		parts[i].size = rand() % 200 + 50;
		parts[i].flag = 0;
	}
	freePart += number;
}

void paBurst(float x, float y, float dx, float dy, float r, float size, unsigned int color) {
	int i;
	i = freePart;
	parts[i].maxlife = rand() % 1000 + 1000 / size;
	parts[i].x = x;
	parts[i].y = y;
	parts[i].dx = dx - size * 0.8 * cos(r) + 0.2 * ((rand() % 1000 - 500) / 500.f);
	parts[i].dy = dy - size * 0.8 * sin(r) + 0.2 * ((rand() % 1000 - 500) / 500.f);
	parts[i].life = parts[i].maxlife;
	parts[i].size = (rand() % 100 + 50) * size;
	parts[i].color = color;
	parts[i].flag = 0;
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
	parts[i].flag = 0;
	freePart++;
	if (freePart >= NBPART)
		freePart = 0;
}

void paLas(float x, float y, float dx, float dy, float len, float r, unsigned int color) {
	int i;
	i = freePart;
	parts[i].maxlife = rand() % 30 + 30;
	parts[i].x = x;
	parts[i].y = y;
	parts[i].dx = dx;
	parts[i].dy = dy;
	parts[i].size = len;
	parts[i].r = r;
	parts[i].life = parts[i].maxlife;
	parts[i].color = color;
	parts[i].flag = PA_LASER;
	freePart++;
	if (freePart >= NBPART)
		freePart = 0;
}

void paUpdate(float dt) {
	int i;
	float c;
	grSetBlendAdd(texture);
	for (i = 0; i < NBPART; i++) {
		if (parts[i].life <= 0)
			continue;
		c = (float) parts[i].life / (float) parts[i].maxlife;
		parts[i].color &= ~0xFF;
		parts[i].color |= (int) (c * 255);
		grSetColor(parts[i].color);
		if (parts[i].flag == PA_LASER) {
			grSetBlendAdd(lastex);
			grBlitLaser(parts[i].x, parts[i].y, parts[i].size,
					parts[i].r, 40.);
			grSetBlendAdd(texture);
		} else {
			grBlitSquare(parts[i].x, parts[i].y, parts[i].size);
		}
		parts[i].x += c * parts[i].dx * dt;
		parts[i].y += c * parts[i].dy * dt;
		parts[i].life -= dt;
	}
}
