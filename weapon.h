/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef WEAPON_H_
#define WEAPON_H_

void weInit(void);
void weLaser(int netid, pos_t *p, float len, float width, float lifetime, unsigned int color, float time);
void weUpdate(float time);

#endif /* WEAPON_H_ */
