/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef TURRET_H_
#define TURRET_H_

#include "ship.h"

void tuAddTurret(ship_t * sh);
void tuUpdate(ship_t *sh, float dt);
void tuSetMove(int netid, signed char *dir, float time);
turret_t *tuCheckTurret(ship_t *sh, pos_t *p, pos_t *ms, float len, float *min);
void tuDamage(turret_t *tu, float dg, float time);

#ifndef DEDICATED
void tuDraw(ship_t * sh, float time);
#else
#define tuDraw
#define tuLoadTurret
#endif

#endif /* TURRET_H_ */
