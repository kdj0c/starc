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
#include "weapon.h"

#define ENG_POWER 1000.
#define DEAD -12345.f
#define RSP_TIME 5000.

struct list_head *ship_head = NULL;

static ship_t *shPlayer = NULL;

static void addShip(ship_t *sh) {
	if (sh->t->flag & SH_MOTHERSHIP)
		list_add(&sh->list, ship_head);
	else
		list_add_tail(&sh->list, ship_head);
}

static void removeShip(ship_t *sh) {
	list_del(&sh->list);
}

ship_t *shGetPlayer(void) {
	return shPlayer;
}

void shSetPlayer(ship_t *sh) {
	shPlayer = sh;
}

void shSetList(struct list_head *list) {
	ship_head = list;
}

ship_t *shCreateShip(char *name, pos_t *pos, int team, int netid, float time) {
	ship_t *newship;

	printf("create new ship[%s], id %d\n", name, netid);

	newship = malloc(sizeof(ship_t));
	memset(newship, 0, sizeof(ship_t));
	newship->t = cfGetShip(name);
	newship->traj.base = *pos;
	newship->pos = *pos;
	newship->traj.type = t_linear;
	newship->traj.basetime = time;
	newship->team = team;
	newship->health = newship->t->maxhealth;
	newship->netid = netid;
	newship->thrusttime = time;

	if (newship->t->numturret)
		tuAddTurret(newship);

	if (newship->t->numparts) {
		int i;

		newship->part = malloc(sizeof(part_t) * newship->t->numparts);
		memset(newship->part, 0, sizeof(part_t) * newship->t->numparts);
		for (i = 0; i < newship->t->numparts; i++)
			newship->part[i].health = newship->t->part->part->maxhealth;
	}
	addShip(newship);
	return newship;
}

int shPostAllShips(float time, void *data) {
	ship_t *sh;
	ev_cr_t *ev;
	int n = 0;

	ev = (ev_cr_t *) data;
	list_for_each_entry(sh, ship_head, list) {
		get_pos(time, &sh->traj, &ev->pos);
		ev->owner = sh->netid;
		ev->control = pl_remote;
		ev->team = sh->team;
		strcpy(ev->shipname, sh->t->name);
//        ev->time = time;
		ev++;
		n++;
	}
	return n;
}

void shFire(int netid, pos_t *p, int weNum, unsigned int weId, float time) {
	ship_t *sh;

	sh = shGetByID(netid);

	/* for turret */
	if (weNum >= MAX_WEAPON) {
		int tu;
		int l;
		l = (weNum - MAX_WEAPON) % MAX_TURRET;
		tu = (weNum - MAX_WEAPON) / MAX_TURRET;
		weFire(netid, p, sh->t->turret[tu].t->laser[l].wt, weId, time);
	} else
		weFire(netid, p, sh->t->laser[weNum].wt, weId, time);
}

int shDetectHit(int netid, pos_t *p, float size, int weId, float time) {
	ship_t *sh;

	list_for_each_entry(sh, ship_head, list) {
		float s;
		pos_t shp;
		vec_t d;
		if (sh->health <= 0)
			continue;
		if (sh->netid == netid)
			continue;
		get_pos(time, &sh->traj, &shp);
		d = vsub(shp.p, p->p);
		s = sh->t->shieldsize / 2.f;

		if (sqnorm(d) > s * s)
			continue;

		if (sh->t->numparts) {
			partpos_t *pt;
			vec_t pp;
			int i;

			for (i = 0; i < sh->t->numparts; i++) {
				if (sh->part[i].health <= 0.)
					continue;

				pt = &sh->t->part[i];
				pp = vmatrix(shp.p, pt->p, sh->pos.r);
				d = vsub(pp, p->p);
				s = pt->part->shieldsize / 2.f;
				if (sqnorm(d) > s * s)
					continue;
				p->v = shp.v;
				evPostHit(netid, sh->netid, i, p, weId, time);
			}
		} else {
			p->v = shp.v;
			evPostHit(netid, sh->netid, 0, p, weId, time);
			return 1;
		}
	}
	return 0;
}

void shFireWeapon(ship_t *sh, pos_t *p, int weNum, float time) {
	pos_t pl;
	weapon_t *las = &sh->t->laser[weNum];

	pl.p = vmatrix(p->p, las->p, p->r);
	pl.r = p->r + las->r;
	pl.v = p->v;
	evPostFire(sh->netid, &pl, weNum, sh->weId, time);
	sh->lastfire[weNum] = time;
	sh->weId++;
	if (sh->weId >= MAX_WEID)
		sh->weId = 0;
}

void shNewTraj(shin_t *in, int netid, float time) {
	ship_t *sh;
	traj_t *t;
	pos_t newbase;

	sh = shGetByID(netid);

	t = &sh->traj;
	get_pos(time, t, &newbase);

	if (sh->in.acceleration != in->acceleration || sh->in.direction != in->direction) {
		if (in->acceleration == 0.f)
			t->type = t_linear;
		else if (in->direction < 0.001f && in->direction > -0.001f)
			t->type = t_linear_acc;
		else
			t->type = t_circle;

		t->base = newbase;
		t->man = sh->t->maniability * in->direction;
/*		if (!sh->in.acceleration) {
			sh->engtime += time - t->basetime;
			if (sh->engtime > ENG_POWER)
				sh->engtime = ENG_POWER;
		}
		if (sh->engtime > 0.)
			t->thrust = sh->t->thrust;
		else
			t->thrust = sh->t->thrust / 3.;
		*/

		t->basetime = time;
		t->thrust = sh->t->thrust * in->acceleration;
	}
	memcpy(&sh->in, in, sizeof(*in));
}

/*
 * TODO Need to fix this wrong algo
 */
void shDisconnect(int clid) {
	ship_t *sh;
	list_for_each_entry(sh, ship_head, list) {
		if (sh->netid >> 8 == clid) {
			printf("Disconnect ship %d\n", sh->netid);
			removeShip(sh);
		}
	}
}

void shDamage(ship_t *sh, float dg, float time) {
	if (sh->health > 0) {
		sh->health -= dg;
		sh->lastdamage = time;
	}
}

void shDamagePart(ship_t *sh, int part, float dg, float time) {
	if (sh->part[part].health > 0) {
		sh->part[part].health -= dg;
		sh->part[part].lastdamage = time;
	}
}

void shDestroy(int netid, int part, float time) {
	ship_t *sh;

	sh = shGetByID(netid);
	if (sh->t->numparts) {
		partpos_t *pt;
		vec_t p;

		pt = &sh->t->part[part];
		p = vmatrix(sh->pos.p, pt->p, sh->pos.r);
		paExplosion(p, sh->pos.v, time);
	} else {
		paExplosion(sh->pos.p, sh->pos.v, time);
		if (sh->health > 0)
			sh->health = 0;
	}
}

void shRespawn(int netid, pos_t *np, int msid, float time) {
	ship_t *sh;
	ship_t *ms;
	sh = shGetByID(netid);
	sh->health = sh->t->maxhealth;
	sh->engtime = ENG_POWER;
	if (msid > 0) {
		ms = shGetByID(msid);
		msRespawn(sh, ms, time);
	} else {
		sh->traj.base = *np;
		sh->traj.basetime = time;
	}
}

void shHit(int owner, int tgid, int partid, pos_t *p, int weId, int server, float time) {
	ship_t *tg;
	float damage;

	damage = weHit(weId, p, server, time);

	tg = shGetByID(tgid);

	if (tg->t->numparts)
		shDamagePart(tg, partid, damage, time);
	else
		shDamage(tg, damage, time);
}

/*
 * ships are considered round (it bounce on shield)
 * and the shock is elastic Formula :
 *         ->    ->  ->
 * k = 2 * AB . (Va -Vb) * m1 * m2 / (AB� * (m1 + m2))
 * ->    ->       ->     ->    ->       ->
 * Va' = Va - k * AB and Vb' = Vb + k * AB
 */
// Minimum bounce if objects collide at very low speed, to avoid multiple collision.
#define MINB 0.30f

void shCollide(int netid1, int netid2, pos_t *p1, pos_t *p2, float time) {
	float k;
	float m1, m2;
	ship_t *sh1;
	ship_t *sh2;
	vec_t d, dv;

	sh1 = shGetByID(netid1);
	sh2 = shGetByID(netid2);
	d = vsub(p1->p, p2->p);
	dv = vsub(p1->v, p2->v);
	m1 = sh1->t->size;
	m2 = sh2->t->size;

	k = 2 * scal(d, dv);
	if (k >= 0.)
		return;
	k /= (norm(d) * norm(d));
	k *= m1 * m2 / (m1 + m2);

	// if both ships are on the same team, lower the bounce effect
	if (sh1->team == sh2->team)
		k /= 4.0f;

	if (k > -MINB && k < 0)
		k = -MINB;

	sh1->traj.basetime = time;
	sh2->traj.basetime = time;
	p1->v = vsub(p1->v, vmul(d, (k / m1)));
	p2->v = vadd(p2->v, vmul(d, (k / m2)));
	sh1->traj.base = *p1;
	sh2->traj.base = *p2;

	if (sh1->team != sh2->team) {
		shDamage(sh1, 100, time);
		shDamage(sh2, 100, time);
	} else {
		shDamage(sh1, 1, time);
		shDamage(sh2, 1, time);
	}
}

#ifndef DEDICATED
#define PA_PER_SECONDS (1000./100.)
void shBurst(ship_t *sh, float time) {
	int i;
	float ttime;

	if (!sh->traj.thrust) {
		sh->thrusttime = time;
		return;
	}
	for (ttime = sh->thrusttime + PA_PER_SECONDS; ttime < time; ttime += PA_PER_SECONDS) {
		for (i = 0; i < sh->t->numburst; i++) {
			pos_t p;
			get_pos(ttime, &sh->traj, &p);
			p.p = vmatrix(p.p, sh->t->burst[i].p, p.r);
			paBurst(&p, sh->t->burst[i].size, sh->t->burst[i].color, ttime);
		}
		sh->thrusttime = ttime;
	}
}
#endif

void shUpdateLocal(float time) {
	ship_t *sh;

	list_for_each_entry(sh, ship_head, list) {
		if (sh->traj.type != t_none) {
			get_pos(time, &sh->traj, &sh->pos);
		}
#ifndef DEDICATED
		if (sh->health <= 0)
			continue;
		shBurst(sh, time);
#endif
	}
}

// avoid two ships to respawn at the same time on a mothership
#define MS_RESPAWN_DELAY 2000.
void shUpdateShips(float time) {
	ship_t *sh;
	int i;

	list_for_each_entry(sh, ship_head, list) {
		if (sh->health == DEAD)
			continue;

		if (sh->health <= 0 && !sh->t->flag | SH_MOTHERSHIP) {
			pos_t np;
			int msid;
			ship_t *ms;
			float rsptime;

			sh->health = DEAD;

			evPostDestroy(sh->netid, 0, time);
			ms = shFindMotherShip(sh->team);
			rsptime = time + RSP_TIME;
			if (ms) {
				msid = ms->netid;
				if (ms->hgRespawn > rsptime - MS_RESPAWN_DELAY)
					rsptime = ms->hgRespawn + MS_RESPAWN_DELAY;
				ms->hgRespawn = rsptime;
			} else
				msid = -1;

			np.p.x = (rand() % 10000 - 5000) * 2.;
			np.p.y = (rand() % 10000 - 5000) * 2.;
			np.r = (rand() % 360 - 180) * M_PI / 180.;
			np.v.x = 0;
			np.v.y = 0;

			evPostRespawn(&np, sh->netid, msid, rsptime);
			continue;
		}
		if (sh->t->numparts) {
			for (i = 0; i < sh->t->numparts; i++) {
				if (sh->part[i].health <= 0 && sh->part[i].health != DEAD) {
					sh->part[i].health = DEAD;
					evPostDestroy(sh->netid, i, time);
				}
			}
		}

		if (sh->in.fire1) {
			int i;
			for (i = 0; i < sh->t->numweapon; i++) {
				if (time + 50. - sh->lastfire[i] > sh->t->laser[i].wt->firerate) {
					float ftime;
					pos_t p;

					ftime = sh->lastfire[i] + sh->t->laser[i].wt->firerate;
					if (ftime < time)
						ftime = time;
					get_pos(ftime, &sh->traj, &p);
					shFireWeapon(sh, &p, i, ftime);
				}
			}
		}
		if (sh->turret)
			tuUpdate(sh, time);
	}
}

void shDetectCollision(float time) {
	ship_t *sh;
	pos_t p1;
	pos_t p2;

	list_for_each_entry(sh, ship_head, list) {
		ship_t *en;
		float s;

		if (sh->health <= 0)
			continue;
		if (sh->t->flag & SH_MOTHERSHIP)
			continue;
		en = sh;
		get_pos(time, &sh->traj, &p1);
		list_for_each_entry_continue(en, ship_head, list) {
			if (en->health <= 0 || (en->t->flag & SH_MOTHERSHIP))
				continue;
			s = (en->t->shieldsize + sh->t->shieldsize) / 2.f;
			s = s * s;
			get_pos(time, &en->traj, &p2);
			if (sqdist(p1.p, p2.p) < s) {
				printf("collision, %d, %d, %f\n", sh->netid, en->netid, sqdist(p1.p, p2.p));
				evPostCollide(sh->netid, en->netid, &p1, &p2, time);
			}
		}
	}
}

ship_t *shFindMotherShip(int team) {
	ship_t *sh;
	list_for_each_entry(sh, ship_head, list) {
		if (sh->team != team)
			continue;
		if (sh->t->flag & SH_MOTHERSHIP)
			return sh;
	}
	return NULL;
}

ship_t *shFindNearestEnemy(ship_t *self) {
	ship_t *sh;
	float min_d = 0.f;
	float d;
	ship_t *nr = NULL;
	list_for_each_entry(sh, ship_head, list) {
		if (sh == self || sh->health <= 0)
			continue;
		if (sh->team == self->team)
			continue;
		if (sh->t->flag & SH_MOTHERSHIP)
			continue;

		d = sqdist(self->pos.p, sh->pos.p);
		if (nr && d > min_d)
			continue;

		nr = sh;
		min_d = d;
	}
	return nr;
}

ship_t *shGetByID(int id) {
	ship_t *sh;
	list_for_each_entry(sh, ship_head, list) {
		if (sh->netid == id) {
			return sh;
		}
	}
	return NULL;
}

void shDrawDebug(ship_t *sh, float time) {
	pos_t pl;
	int l;

	static texc_t tex;

	if (!tex.h)
		cfGetTexture("star", &tex);

	for (l = 0; l < sh->t->numweapon; l++) {
		weapon_t *las = &sh->t->laser[l];

		pl.p = vmatrix(sh->pos.p, las->p, sh->pos.r);
		pl.r = sh->pos.r + las->r;
		grBatchAddRot(pl.p, pl.r, &tex, 0xFFFFFFFF);
	}

	for (l = 0; l < sh->t->numburst; l++) {
		burst_t *b = &sh->t->burst[l];

		pl.p = vmatrix(sh->pos.p, b->p, sh->pos.r);
		grBatchAddRot(pl.p, sh->pos.r, &tex, 0xFFFFFFFF);
	}

}

#ifndef DEDICATED
void shDrawPart(ship_t *sh, float time) {
	int i;
	partpos_t *pt;

	for (i = 0; i < sh->t->numparts; i++) {
		vec_t p;
		float r;
		unsigned int color = 0xFFFFFFFF;

		pt = &sh->t->part[i];
		p = vmatrix(sh->pos.p, pt->p, sh->pos.r);
		r = sh->pos.r + pt->r;

        if (sh->part[i].health <= 0)
            color = 0x20FFFFFF;

		grBatchAddRot(p, r, &pt->part->tex, color);
	}
}

#define SHIELD_FADE 800.
void shDrawPartShields(ship_t *sh, float time) {
	int i;
	partpos_t *pt;

	for (i = 0; i < sh->t->numparts; i++) {
		vec_t p;
		unsigned char fade;

		if (sh->part[i].health <= 0)
			continue;

		if (time - sh->part[i].lastdamage >= SHIELD_FADE)
			continue;

		fade = (unsigned char) (255 - 255 * ((time - sh->part[i].lastdamage) / SHIELD_FADE));

		pt = &sh->t->part[i];
		p = vmatrix(sh->pos.p, pt->p, sh->pos.r);

		grBatchAdd(p, sh->t->part[i].part->shieldsize * M_SQRT1_2, 0, &sh->t->shieldtexture, sh->t->shieldcolor | (fade << 24));
	}
}

void shDrawShips(float time) {
	ship_t *sh;

	list_for_each_entry(sh, ship_head, list) {
		if (sh->health <= 0)
			continue;
		get_pos(time, &sh->traj, &sh->pos);

		if (!sh->t->numparts) {
			grBatchAddRot(sh->pos.p, sh->pos.r, &sh->t->texture, 0xFFFFFFFF);
			shDrawDebug(sh, time);
		} else {
			shDrawPart(sh, time);
		}
		if (sh->t->numturret) {
			tuDraw(sh, time);
		}

	}
	grBatchDraw();
}

void shDrawShields(float time) {
	ship_t *sh;

	grSetBlendAdd();

	list_for_each_entry(sh, ship_head, list) {
		if (sh->health <= 0)
			continue;

		if (sh->t->numparts) {
			shDrawPartShields(sh, time);
			continue;
		}

		if (time - sh->lastdamage < SHIELD_FADE) {
			unsigned char fade;

			fade = (unsigned char) (255 - 255 * ((time - sh->lastdamage) / SHIELD_FADE));
			grBatchAdd(sh->pos.p, sh->t->shieldsize * M_SQRT1_2, 0, &sh->t->shieldtexture, sh->t->shieldcolor | (fade << 24));
		}
	}
	grBatchDraw();
}
#endif
