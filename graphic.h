/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef GRAPHIC_H_
#define GRAPHIC_H_

#include <SDL2/SDL.h>
#include "vec.h"

#define WIDTH_UNIT 20000

typedef struct {
	int fullscreen;
	int width;
	int heigh;
} grconf_t;

typedef struct {
	float texc[8];
	float w;
	float h;
	int index;
} texc_t;

#ifndef DEDICATED

void grInit(grconf_t *c);
void grInitShader(void);
void grInitQuad(void);

unsigned int grLoadTextureArray(void);
void grSetBlendAdd(void);
void grSetBlend(void);
void grSetColor(unsigned int color);
void grSetShadow(float c);
void grBlitSquare(vec_t p, float size, int i, float *texc);
void grBlitRot(vec_t p, float r, float size, int i, float *texc);
void grBlitRot2(vec_t p, float r, texc_t *tex);
void grBlit(vec_t p, float a, float b, int i, float *texc);
void grChangeview(float x, float y, float r, float scale);
void grReshape(int width, int height);
void grBlitLaser(float x, float y, float len, float r, float width);
void grDrawHUD(float health);
void grSwap(void);
#else
#define grSetColor(color)
#endif

#endif /* GRAPHIC_H_ */
