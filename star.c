/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include "graphic.h"

typedef struct {
	float x;
	float y;
	float size;
	float c;
} star_t;

#define MAXST 20
#define random1 ((float) (rand() % 1000)  / 500.f)

star_t star[MAXST][MAXST];
int curx = 0;
int cury = 0;
unsigned int tex = 0;

static void newrandstar(int cx, int cy) {
	star[cx][cy].x = random1;
	star[cx][cy].y = random1;
	star[cx][cy].size = random1 * 50 + 50;
	star[cx][cy].c = random1;
}

void stUpdate(float x, float y) {
	int ncx, ncy;
	int dx, dy;
	int cx, cy;
	static int initdone = 0;

	if (!initdone) {
		for (cx = 0; cx < MAXST; cx++) {
			for (cy = 0; cy < MAXST; cy++) {
				newrandstar(cx, cy);
			}
		}
		tex = grLoadTexture("img/star.png");
		initdone = 1;
		return;
	}

	ncx = (x - 10000) / 1000;
	ncy = (y - 10000) / 1000;
	dx = ncx - curx;
	dy = ncy - cury;

	if (dx > 0) {
		for (cy = 0; cy < MAXST; cy++) {
			for (cx = dx; cx < MAXST; cx++) {
				star[cx - dx][cy] = star[cx][cy];
			}
			for (cx = MAXST - dx; cx < MAXST; cx++) {
				newrandstar(cx, cy);
			}
		}
	} else if (dx < 0) {
		for (cy = 0; cy < MAXST; cy++) {
			for (cx = MAXST; cx >= -dx; cx--) {
				star[cx][cy] = star[cx + dx][cy];
			}
			for (cx = 0; cx < -dx; cx++)
				newrandstar(cx, cy);
		}
	}
	if (dy > 0) {
		for (cx = 0; cx < MAXST; cx++) {
			for (cy = dy; cy < MAXST; cy++) {
				star[cx][cy - dy] = star[cx][cy];
			}
			for (cy = MAXST - dy; cy < MAXST; cy++)
				newrandstar(cx, cy);
		}
	} else if (dy < 0) {
		for (cx = 0; cx < MAXST; cx++) {
			for (cy = MAXST; cy >= -dy; cy--) {
				star[cx][cy] = star[cx][cy + dy];
			}
			for (cy = 0; cy < -dy; cy++)
				newrandstar(cx, cy);
		}
	}
	curx = ncx;
	cury = ncy;
}

void stBlit(void) {
	int cx, cy;
	float x, y;
	grSetBlend(tex);
	for (cx = 0; cx < MAXST; cx++) {
		for (cy = 0; cy < MAXST; cy++) {
			x = (curx + cx + star[cx][cy].x) * 1000;
			y = (cury + cy + star[cx][cy].y) * 1000;
			grBlitSquare(x, y, star[cx][cy].size, star[cx][cy].c);
		}
	}
}
