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
#include "event.h"

extern float frametime;

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

ship_t * shCreateShip(char * name, pos_t *pos, int team, int netid) {
	ship_t * newship;

	newship = malloc(sizeof(ship_t));
	memset(newship, 0, sizeof(ship_t));
	newship->t = cfGetShip(name);
	newship->traj.base = *pos;
	newship->pos = *pos;
	newship->traj.type = t_linear;
	newship->traj.basetime = 0.0;
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
//				paExplosion(shc->x, shc->y, shc->dx, shc->dy, 6.f, 5000, sh->t->burst[0].color);
//			memcpy(&sh->pos.p.x, shc, size);
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

void shNewTraj(shin_t *in, int netid,  float time) {
	ship_t *sh;
	traj_t *t;
	pos_t newbase;

	sh = shGetByID(netid);

    t = &sh->traj;
    get_pos(time, t, &newbase);
    if (!in->acceleration)
        t->type = t_linear;
    else if (!in->direction)
        t->type = t_linear_acc;
    else
        t->type = t_circle;
    t->basetime = time;
    t->base = newbase;
    t->man = sh->t->maniability * in->direction;
    t->thrust = sh->t->thrust;
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
    int msid;
    ship_t *ms;
	sh->health -= dg;
	sh->drawshield = 500;
	if (sh->health <= 0 && sh->health + dg > 0) {
	    pos_t np;

	    sh->health = 0;
	    evPostDestroy(sh->netid, frametime);
	    ms = shFindMotherShip(sh->team);
	    if (ms)
            msid = ms->netid;
        else
            msid = -1;

        np.p.x = (rand() % 10000 - 5000) * 2.;
        np.p.y = (rand() % 10000 - 5000) * 2.;
        np.r = (rand() % 360 - 180) * M_PI / 180.;
        np.v.x = 0;
        np.v.y = 0;
        sh->drawshield = 0;
	    evPostRespawn(&np, sh->netid, msid, frametime + 5000.);
	}
}

void shDestroy(int netid) {
    ship_t *sh;

    sh = shGetByID(netid);
    paExplosion(sh->pos.p, sh->pos.v, 3.f, 2000, sh->t->burst[0].color);
    sh->health = 0;
}

void shRespawn(int netid, pos_t *np, int msid, float time) {
    ship_t *sh;
    ship_t *ms;
    sh = shGetByID(netid);
    if (msid > 0) {
        ms = shGetByID(msid);
        msRespawn(sh, ms, time);
    } else {
        sh->health = sh->t->maxhealth;
        sh->traj.base = *np;
        sh->traj.basetime = time;
    }
}

void shFireLaser(pos_t p, ship_t *sh, laser_t *las, float dt) {
    ship_t * en;
	ship_t * tc = NULL;
	float len, min;

	min = LASER_RANGE;
	list_for_each_entry(en, &ship_head, list) {
		float s;
		vec_t d, t;
		if (en->health <= 0 || en == sh || (en->t->flag & SH_MOTHERSHIP))
			continue;
        d = vsub(en->pos.p, p.p);
        t = vmatrix1(d, p.r);

		s = en->t->shieldsize / 2.f;

		if (t.x > 0 && t.x < LASER_RANGE + s && t.y > -s && t.y < s) {
			len = t.x - sqrt(s * s - t.y * t.y);
			if (len < min) {
				min = len;
				tc = en;
			}
		}
	}
	if(tc) {
	    vec_t tmp;
		shDamage(tc, dt);
		tmp = vadd(p.p, vangle(min, p.r));
		paLaser(tmp, tc->pos.v, las->color);
	}
	paLas(p, min, las->color);
}


void shShipFireLaser(ship_t * sh, laser_t * las, float dt) {
	pos_t p;

	p.p = vmatrix(sh->pos.p, las->p, sh->pos.r);
	p.r = sh->pos.r + las->r;
	p.v = sh->pos.v;
    shFireLaser(p, sh, las, dt);
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
	k = 2 * (dx * (sh->pos.v.x - en->pos.v.x) + dy * (sh->pos.v.y - en->pos.v.y));
	if(k == 0.)
		return;
	k /= (dx * dx + dy * dy);
	k *= m1 * m2 / (m1 + m2);
	sh->pos.v.x -= k * dx / m1;
	sh->pos.v.y -= k * dy / m1;
	en->pos.v.x += k * dx / m2;
	en->pos.v.y += k * dy / m2;

	shDamage(sh, 100);
	shDamage(en, 100);
	/* be sure ships are far enough before next collision test */
/*	sh->pos.p.x += sh->pos.v.x * 30;
	sh->pos.p.y += sh->pos.v.y * 30;
	en->x += en->dx * 30;
	en->y += en->dy * 30;*/
}

void shBurst(ship_t *sh) {
	int i;
	for(i=0;i<sh->t->numburst;i++) {
	    pos_t p;
	    p.r = sh->pos.r;
	    p.v = sh->pos.v;
	    p.p = vmatrix(sh->pos.p, sh->t->burst[i].p, sh->pos.r);
		paBurst(p, sh->t->burst[i].size, sh->t->burst[i].color);
	}
}

void shUpdateShips(float time) {
	ship_t * sh;
	int l;
    static float prevup = 0.;
    float dt;

    dt = time - prevup;
    prevup = time;

	list_for_each_entry(sh, &ship_head, list) {
		if (sh->health <= 0)
			continue;
		if (sh->drawshield > 0)
			sh->drawshield -= dt;
	}

	list_for_each_entry(sh, &ship_head, list) {
		if (sh->health <= 0)
			continue;

        if (sh->traj.type != t_none) {
            get_pos(time, &sh->traj, &sh->pos);
            sh->pos.p.x = sh->pos.p.x;
            sh->pos.p.y = sh->pos.p.y;
            sh->pos.r = sh->pos.r;
        }
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
			dx = en->pos.p.x - sh->pos.p.x;
			dy = en->pos.p.y - sh->pos.p.y;
			s = (en->t->shieldsize + sh->t->shieldsize) / 2.f;
			s = s * s;
			if (dx * dx + dy * dy < s) {
				shCollide(sh, en, dx, dy);
			}
		}
	}
}

void shUpdateRespawn(float dt) {
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
	float d;
	ship_t * nr = NULL;
	list_for_each_entry(sh, &ship_head, list) {
		if (sh == self || sh->health <= 0)
			continue;
		if (sh->team == self->team)
			continue;

        d = sqdist(self->pos.p, sh->pos.p);
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
		memcpy(shc, &sh->pos.p.x, sizeof(*shc));
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
		memcpy(&shn->x, &sh->pos.p.x, sizeof(shipcore_t));
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
		grBlitRot(sh->pos.p.x, sh->pos.p.y, sh->pos.r, sh->t->size);
		if (sh->drawshield > 0) {
			grSetBlendAdd(sh->t->shieldtex);
			grBlit(sh->pos.p.x, sh->pos.p.y, sh->t->shieldsize * M_SQRT1_2, 0);
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
		dx = sh->pos.p.x - pl->pos.p.x;
		dy = sh->pos.p.y - pl->pos.p.y;
		if(dx * dx + dy * dy < LASER_RANGE * LASER_RANGE * 4)
			continue;
		r = atan2(dx, -dy) - pl->pos.r;
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

