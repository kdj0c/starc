/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef WEAPON_H_
#define WEAPON_H_

enum {
	WE_LASER,
	WE_MISSILE,
};

typedef struct {
	char name[16];
	texc_t texture;
	/* diameter */
	float size;
	float speed;
	float lifetime;
	float firerate;
	int damage;
	unsigned int color;
	int type;
} weapontype_t;

void weInit(void);
int weGetFree(void);
void weFire(int netid, pos_t *p, weapontype_t *wt, float time);
void weUpdate(float time);
float weHit(int id, pos_t *p, float time);

#ifndef DEDICATED
void weDraw(float time);
#endif
#endif /* WEAPON_H_ */
