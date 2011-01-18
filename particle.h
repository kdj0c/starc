/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef PARTICLE_H_
#define PARTICLE_H_

#ifndef DEDICATED
void paInit(void);
void paExplosion(float x, float y, float dx, float dy, float v, int number, unsigned int color);
void paUpdate(float dt);
void paBurst(float x, float y, float dx, float dy, float r, unsigned int color);
void paLaser(float x, float y, float dx, float dy, unsigned int color);
void paLas(float x, float y, float dx, float dy, float len, float r, unsigned int color);
#else
#define paInit()
#define paExplosion(x, y, dx, dy, v, number, color)
#define paUpdate(dt)
#define paBurst(x, y, dx, dy, r, color)
#define paLaser(x, y, dx, dy, color)
#define paLas(x, y, dx, dy, len, r, color)
#endif

#endif /* PARTICLE_H_ */
