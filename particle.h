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
void paExplosion(vec_t p, vec_t v, float s, int number, unsigned int color);
void paUpdate(float dt);
void paBurst(pos_t p, float size, unsigned int color);
void paLaser(vec_t p, vec_t v, unsigned int color);
void paLas(pos_t p, float len, unsigned int color);
#else
#define paInit()
#define paExplosion(p, v, s, number, color)
#define paUpdate(dt)
#define paBurst(p, size, color)
#define paLaser(p, v, color)
#define paLas(p, len, color)
#endif

#endif /* PARTICLE_H_ */
