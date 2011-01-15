/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef GRAPHIC_H_
#define GRAPHIC_H_

#define WIDTH_UNIT 20000

#ifndef DEDICATED
unsigned int grLoadTexture(char * filename);
void grSetBlendAdd(unsigned int text);
void grSetBlend(unsigned int text);
void grSetColor(unsigned int color);
void grSetShadow(float c);
void grBlitSquare(float x,float y, float size);
void grBlitRot(float x, float y, float r, float size);
void grBlit(float x,float y, float a, float b);
void grDrawLine(float x1, float y1, float x2, float y2);
void grChangeview(float x, float y, float r, float scale);
void grReshape(int width, int height);
void grBlitLaser(float x, float y, float len, float r, float width);
#else
#define grLoadTexture(f)
#define grSetColor(color)
#define grDrawLine(x1, y1, x2, y2)

#endif

#endif /* GRAPHIC_H_ */
