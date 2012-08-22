/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include <stdio.h>

#include "graphic.h"
#include "ship.h"
#include "particle.h"
#include "config.h"
#include "turret.h"
#include "mothership.h"

#ifndef DEDICATED
static int hudarrowtex = 0;
#endif

LIST_HEAD(ship_head);

static void addShip(ship_t * sh) {
	if (sh->t->flag & SH_MOTHERSHIP)
		list_add(&sh->list, &ship_head);
	else
		list_add_tail(&sh->list, &ship_head);
}

static void removeShip(ship_t * sh) {
	list_del(&sh->list);
}

void shLoadShipType(void) {
}

#ifndef DEDICATED
void shLoadShip(void) {
	ship_t * sh;

	list_for_each_entry(sh, &ship_head, list) {
		if(!sh->t->tex)
			sh->t->tex = grLoadTexture(sh->t->imgfile);
		if(!sh->t->shieldtex)
			sh->t->shieldtex = grLoadTexture(sh->t->shieldfile);
	}
	hudarrowtex = grLoadTexture("img/arrow.png");
}
#endif

ship_t * shCreateShip(char * name, float x, float y, float r, int team, int netid) {
	ship_t * newship;

	newship = malloc(sizeof(ship_t));
	memset(newship, 0, sizeof(ship_t));
	newship->t = cfGetShip(name);
	newship->x = x;
	newship->y = y;
	newship->r = r;
	newship->team = team;
	newship->health = newship->t->maxhealth;
	newship->netid = netid;

	if(newship->t->numturret)
		tuAddTurret(newship);

	addShip(newship);
	return newship;
}

ship_t * shCreateRemoteShip(shipcorename_t * shn) {
	ship_t * newship;
	ship_t * sh;

	list_for_each_entry(sh, &ship_head, list) {
		if (sh->netid == shn->netid) {
			/* we have already this ship no need to create a new one !*/
			shSync((shipcore_t *) &shn->x, 0);
			return sh;
		}
	}

	newship = malloc(sizeof(ship_t));
	memset(newship, 0, sizeof(ship_t));
	newship->t = cfGetShip(shn->typename);
	if(!newship->t) {
		printf("can't create ship of type '%s'\n", shn->typename);
		free(newship);
		return NULL;
	}
	memcpy(newship, &shn->x, sizeof(shipcore_t));
	addShip(newship);
	return newship;
}

/*
 * Synchronize local ship with server
 * don't overwrite inputs if it is a local ship
 */
void shSync(shipcore_t * shc, int local) {
	ship_t * sh;
	int size = sizeof(*shc);
	if (local)
		size -= sizeof(shin_t);
	list_for_each_entry(sh, &ship_head, list) {
		if (sh->netid == shc->netid) {
			/* if ship died from last update, make an explosion */
			if(sh->health > 0 && shc->health <= 0)
				paExplosion(shc->x, shc->y, shc->dx, shc->dy, 6.f, 5000, sh->t->burst[0].color);
			memcpy(&sh->x, shc, size);
			return;
		}
	}
}

void shSetInput(shin_t * in, int netid) {
	ship_t * sh;
	list_for_each_entry(sh, &ship_head, list) {
		if (sh->netid == netid) {
			memcpy(&sh->in, in, sizeof(shin_t));
			return;
		}
	}
}
/*
 * TODO Need to fix this wrong algo
 */
void shDisconnect(int clid) {
	ship_t * sh;
	list_for_each_entry(sh, &ship_head, list) {
		if(sh->netid >> 8 == clid) {
			printf("Disconnect ship %d\n", sh->netid);
			removeShip(sh);
		}
	}
}

void shDamage(ship_t * sh, float dg) {
	sh->health -= dg;
	sh->drawshield = 500;
	if (sh->health <= 0 && sh->health + dg > 0) {
		paExplosion(sh->x, sh->y, sh->dx, sh->dy, 3.f, 2000, sh->t->burst[0].color);
	}
	if (sh->health < 0)
		sh->health = 0;
}

void shFireLaser(float x, float y, float r, float dx, float dy, laser_t *las,
		float dt, ship_t *self) {
	ship_t * en;
	ship_t * tc = NULL;
	float len, min;

	min = LASER_RANGE;
	list_for_each_entry(en, &ship_head, list) {
		float dx, dy, tx, ty, s;
		if (en->health <= 0 || en == self || (en->t->flag & SH_MOTHERSHIP))
			continue;
		dx = en->x - x;
		dy = en->y - y;
		tx = dx * cos(r) + dy * sin(r);
		ty = dx * sin(r) - dy * cos(r);
		s = en->t->shieldsize / 2.f;

		if (tx > 0 && tx < LASER_RANGE + s && ty > -s && ty < s) {
			len = tx - sqrt(s * s - ty * ty);
			if (len < min) {
				min = len;
				tc = en;
			}
		}
	}
	if(tc) {
		shDamage(tc, dt);
		paLaser(x + min * cos(r), y + min * sin(r), tc->dx, tc->dy, las->color);
	}
	paLas(x, y, dx, dy, min, r, las->color);
}

void shShipFireLaser(ship_t * sh, laser_t * las, float dt) {
	float x, y, r;

	x = sh->x + las->x * cos(sh->r) + las->y * sin(sh->r);
	y = sh->y + las->x * sin(sh->r) - las->y * cos(sh->r);
	r = sh->r + las->r;
	shFireLaser(x, y, r, sh->dx, sh->dy, las, dt, sh);
}

/*
 * ships are considered round (it bounce on shield)
 * and the shock is elastic Formula :
 *         ->    ->  ->
 * k = 2 * AB . (Va -Vb) * m1 * m2 / (AB² * (m1 + m2))
 * ->    ->       ->     ->    ->       ->
 * Va' = Va - k * AB and Vb' = Vb + k * AB
 */
void shCollide(ship_t * sh, ship_t * en, float dx, float dy) {
	float k;
	float m1, m2;

	m1 = sh->t->size;
	m2 = en->t->size;
	k = 2 * (dx * (sh->dx - en->dx) + dy * (sh->dy - en->dy));
	if(k == 0.)
		return;
	k /= (dx * dx + dy * dy);
	k *= m1 * m2 / (m1 + m2);
	sh->dx -= k * dx / m1;
	sh->dy -= k * dy / m1;
	en->dx += k * dx / m2;
	en->dy += k * dy / m2;

	shDamage(sh, 100);
	shDamage(en, 100);
	/* be sure ships are far enough before next collision test */
/*	sh->x += sh->dx * 30;
	sh->y += sh->dy * 30;
	en->x += en->dx * 30;
	en->y += en->dy * 30;*/
}

void shBurst(ship_t *sh) {
	float x,y;
	int i;
	for(i=0;i<sh->t->numburst;i++) {
		x = sh->x + sh->t->burst[i].x * cos(sh->r) + sh->t->burst[i].y * sin(sh->r);
		y = sh->y + sh->t->burst[i].x * sin(sh->r) - sh->t->burst[i].y * cos(sh->r);
		paBurst(x, y, sh->dx, sh->dy, sh->r, sh->t->burst[i].size, sh->t->burst[i].color);
	}
}

void shUpdateShips(float dt) {
	ship_t * sh;
	int l;

	list_for_each_entry(sh, &ship_head, list) {
		if (sh->health <= 0)
			continue;
		if (sh->drawshield > 0)
			sh->drawshield -= dt;
	}

	list_for_each_entry(sh, &ship_head, list) {
		if (sh->health <= 0)
			continue;
		if (sh->in.direction) {
			sh->r += sh->in.direction * dt * sh->t->maniability;
		}
		if (sh->in.acceleration) {
			sh->dx += sh->t->thrust * dt * cos(sh->r);
			sh->dy += sh->t->thrust * dt * sin(sh->r);
		}
		sh->x += sh->dx * dt;
		sh->y += sh->dy * dt;
		if (sh->in.acceleration)
			shBurst(sh);

		if (sh->in.fire1) {
			for (l = 0; l < sh->t->numlaser; l++) {
				shShipFireLaser(sh, &sh->t->laser[l], dt);
			}
		}
		if(sh->turret)
			tuUpdate(sh, dt);
	}

}

void shDetectCollision(void) {
	ship_t * sh;
	list_for_each_entry(sh, &ship_head, list) {
		ship_t * en;
		float dx, dy, s;
		if (sh->health <=0)
			continue;
		if (sh->t->flag & SH_MOTHERSHIP)
			continue;
		en = sh;
		list_for_each_entry_continue(en, &ship_head, list) {
			if (en->health <= 0 || (en->t->flag & SH_MOTHERSHIP))
				continue;
			dx = en->x - sh->x;
			dy = en->y - sh->y;
			s = (en->t->shieldsize + sh->t->shieldsize) / 2.f;
			s = s * s;
			if (dx * dx + dy * dy < s) {
				shCollide(sh, en, dx, dy);
			}
		}
	}
}

void shUpdateRespawn(float dt) {
	ship_t * sh;

	list_for_each_entry(sh, &ship_head, list) {
		if (sh->health > 0)
			continue;
		sh->health -= dt;

		if (sh->health < -5000.) {
			msRespawn(sh);
			/*
			sh->health = sh->t->maxhealth;
			sh->x = (rand() % 10000 - 5000) * 2.;
			sh->y = (rand() % 10000 - 5000) * 2.;
			sh->r = (rand() % 360 - 180) * M_PI / 180.;
			sh->dx = 0;
			sh->dy = 0;
			sh->drawshield = 0;*/
		}
	}
}

ship_t * shFindMotherShip(int team) {
	ship_t * sh;
	list_for_each_entry(sh, &ship_head, list) {
		if (sh->team != team)
			continue;
		if (sh->t->flag & SH_MOTHERSHIP)
			return sh;
	}
	return NULL;
}

ship_t * shFindNearestEnemy(ship_t * self) {
	ship_t * sh;
	float min_d;
	float dx,dy,d;
	ship_t * nr = NULL;
	list_for_each_entry(sh, &ship_head, list) {
		if (sh == self || sh->health <= 0)
			continue;
		if (sh->team == self->team)
			continue;
		dx = self->x - sh->x;
		dy = self->y - sh->y;
		d = dx * dx + dy * dy;

		if (nr && d > min_d)
			continue;

		nr = sh;
		min_d = d;
	}
	return nr;
}
/*
 * Serialize all ships structure, to be sent by network
 * for every server update
 */
int shSerialize(shipcore_t * data) {
	ship_t * sh;
	shipcore_t * shc;
	int size = 0;

	shc = data;
	list_for_each_entry(sh, &ship_head, list) {
		memcpy(shc, &sh->x, sizeof(*shc));
		shc++;
		size += sizeof(*shc);
	}
	return size;
}

/*
 * Serialize all ships structure, and add typename and any
 * static information, to be sent by network, only once per ship
 */
int shSerializeOnce(shipcorename_t * data) {
	ship_t * sh;
	shipcorename_t * shn;
	int size = 0;

	shn = data;
	list_for_each_entry(sh, &ship_head, list) {
		memcpy(&shn->x, &sh->x, sizeof(shipcore_t));
		strcpy(shn->typename,sh->t->name);
		shn++;
		size += sizeof(*shn);
	}
	return size;
}

ship_t *shGetByID(int id) {
	ship_t *sh;
	list_for_each_entry(sh, &ship_head, list) {
		if (sh->netid == id) {
			return sh;
		}
	}
	return NULL;
}

#ifndef DEDICATED
void shDrawShips(void) {
	ship_t * sh;
	list_for_each_entry(sh, &ship_head, list) {
		if(sh->health <= 0)
			continue;
		grSetBlend(sh->t->tex);
		grBlitRot(sh->x, sh->y, sh->r, sh->t->size);
		if (sh->drawshield > 0) {
			grSetBlendAdd(sh->t->shieldtex);
			grBlit(sh->x, sh->y, sh->t->shieldsize * M_SQRT1_2, 0);
		}
		if (sh->t->numturret) {
			tuDraw(sh);
		}
	}
}

void shDrawShipHUD(ship_t * pl) {
	ship_t * sh;
	float dx,dy,r,x,y;
	grSetBlend(hudarrowtex);
	list_for_each_entry(sh, &ship_head, list) {
		if(sh->health <= 0 || sh  == pl)
			continue;
		dx = sh->x - pl->x;
		dy = sh->y - pl->y;
		if(dx * dx + dy * dy < LASER_RANGE * LASER_RANGE * 4)
			continue;
		r = atan2(dx, -dy) - pl->r;
		x = 800 / 2. + 350. * cos(r);
		y = 600. / 4. + 350 * sin(r);
		if (pl->team == sh->team)
			grSetColor(0x0000FF80);
		else
			grSetColor(0xFF000080);
		grBlitRot(x, y, r, 10);
	}
}
#endif

