/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef GRAPHIC_H_
#define GRAPHIC_H_
#include <GL/glut.h>

#define WIDTH_UNIT 10000

GLuint grLoadTexture(char * filename);
void grSetBlendAdd(GLuint text);
void grSetBlend(GLuint text);
void grSetColor(unsigned int color);
void grBlitSquare(float x,float y, float size);
void grBlitRot(float x, float y, float r, float size);
void grBlit(float x,float y, float a, float b);
void grDrawLine(float x1, float y1, float x2, float y2);
void grChangeview(float x, float y, float r, float scale);
void grReshape(int width, int height);

#endif /* GRAPHIC_H_ */
