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
    int index;
    float texc[8];
} texc_t;

#ifndef DEDICATED

void grInit(grconf_t *c);
void grInitShader(void);
void grInitQuad(void);

unsigned int grLoadTexture(char * filename);
unsigned int grLoadTextureArray(char * filename, int rows, int colomns);
void grSetBlendAdd(unsigned int text);
void grSetBlend(unsigned int text);
void grSetColor(unsigned int color);
void grSetShadow(float c);
void grBlitSquare(vec_t p, float size, int i, float *texc);
void grBlitRot(vec_t p, float r, float size, int i, float *texc);
void grBlit(vec_t p, float a, float b, int i, float *texc);
void grDrawLine(float x1, float y1, float x2, float y2);
void grChangeview(float x, float y, float r, float scale);
void grReshape(int width, int height);
void grBlitLaser(float x, float y, float len, float r, float width);
void grDrawHUD(float health);
void grSwap(void);
#else
#define grLoadTexture(f)
#define grSetColor(color)
#define grDrawLine(x1, y1, x2, y2)

#endif

#endif /* GRAPHIC_H_ */
