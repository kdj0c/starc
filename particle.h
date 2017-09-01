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
void paExplosion(vec_t p, vec_t v, float s, int number, unsigned int color, float time);
void paDraw(float time);
void paDrawExplosion(float time);
void paBurst(pos_t *p, float size, unsigned int color, float time);
void paLaserHit(vec_t p, vec_t v, unsigned int color, float time);
#else
#define paInit()
#define paExplosion(p, v, s, number, color, time)
#define paUpdate(dt)
#define paBurst(p, size, color, time)
#endif

#endif /* PARTICLE_H_ */
