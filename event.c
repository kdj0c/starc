/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "event.h"
#include "ship.h"
#include "ai.h"
#include "weapon.h"
#include "turret.h"
#include "network.h"
#include "gametime.h"
#include "save.h"
#include "server.h"

LIST_HEAD(old_event);
LIST_HEAD(act_event);

void evPostTrajEv(shin_t *in, int owner) {
	ev_tr_t ev;

	ev.owner = owner;
	ev.in = *in;
	evPostEventNow((void *) &ev, sizeof(ev), ev_newtraj);
}

void evPostDestroy(int netid, int part, float time) {
	ev_ds_t ev;

	ev.owner = netid;
	ev.part = part;
	evPostEventLocal(time, (void *) &ev, sizeof(ev), ev_destroyed);
}

void evPostRespawn(pos_t *newp, int netid, int msid, float time) {
	ev_rp_t ev;
	ev.owner = netid;
	ev.ms = msid;
	ev.newpos = *newp;
	evPostEvent(time, (void *) &ev, sizeof(ev), ev_respawn);
}

void evPostCreateShip(char *name, pos_t *p, int team, int netid, int control) {
	ev_cr_t ev;
	float time;

	ev.owner = netid;
	ev.control = control;
	ev.pos = *p;
	ev.team = team;
	memset(ev.shipname, 0, sizeof(ev.shipname));
	strcpy(ev.shipname, name);
	time = gtGetTime();

	sePostEventToServer(time, (void *) &ev, sizeof(ev), ev_newship);
	if (control == pl_ai)
		ev.control = pl_remote;
	else
		ev.control = pl_local;

	evPostEventLocal(time, (void *) &ev, sizeof(ev), ev_newship);
}

void evPostFire(int owner, pos_t *p, int weNum, unsigned int weId, float time) {
	ev_fi_t ev;

	ev.owner = owner;
	ev.p = *p;
	ev.weNum = weNum;
	ev.weId = owner + weId;
	evPostEventLocal(time, (void *) &ev, sizeof(ev), ev_fire);
}

void evPostHit(int owner, int target, int part, pos_t *p, int weId, float time) {
	ev_hi_t ev;

	ev.owner = owner;
	ev.target = target;
	ev.part = part;
	ev.p = *p;
	ev.weId = weId;
	evPostEvent(time, (void *) &ev, sizeof(ev), ev_hit);
}

void evPostTurret(int owner, float *dir, float time) {
	ev_tu_t ev;

	ev.owner = owner;
	memcpy(ev.direction, dir, sizeof(ev.direction));
	evPostEvent(time, (void *) &ev, sizeof(ev), ev_turret);
}

void evPostCollide(int owner1, int owner2, pos_t *p1, pos_t *p2, float time) {
	ev_co_t ev;
	ev.owner1 = owner1;
	ev.owner2 = owner2;
	ev.p1 = *p1;
	ev.p2 = *p2;
	evPostEvent(time, (void *) &ev, sizeof(ev), ev_collide);
}

void evPostEventNow(void *data, int size, event_e type) {
	float time;
	time = gtGetTime();
	evPostEvent(time, data, size, type);
}

void evPostEventLocal(float time, void *data, int size, event_e type) {
	ev_t *new;
	ev_t *prec;
	new = malloc(sizeof(*new) + size);
	new->type = type;
	new->time = time;
	new->size = size;
	if (size && data)
		memcpy(new->data, data, size);

	saSaveEvent(new);

	if (list_empty(&act_event)) {
		list_add(&new->list, &act_event);
		return;
	}
	list_for_each_entry(prec, &act_event, list) {
		if (prec->time > time)
			break;
	}
	list_add_tail(&new->list, &prec->list);
}

void evPostEvent(float time, void *data, int size, event_e type) {
	evPostEventLocal(time, data, size, type);
	sePostEventToServer(time, data, size, type);
}

void evDoEvent(ev_t *ev, int server) {
//    printf("do event %d, %f\n", ev->type, ev->time);
	switch (ev->type) {
	case ev_newship:
	{
		ev_cr_t *cr;
		ship_t *sh;
		cr = (ev_cr_t *) ev->data;
		sh = shCreateShip(cr->shipname, &cr->pos, cr->team, cr->owner, ev->time);
		if (cr->control == pl_ai)
			aiCreate(sh);
		else if (cr->control == pl_local)
			shSetPlayer(sh);
	}
		break;
	case ev_newtraj:
	{
		ev_tr_t *tr;
		tr = (ev_tr_t *) ev->data;
		shNewTraj(&tr->in, tr->owner, ev->time);
	}
		break;
	case ev_destroyed:
	{
		ev_ds_t *ds;
		ds = (ev_ds_t *) ev->data;
		shDestroy(ds->owner, ds->part, ev->time);
		printf("destroyed %d\n", ds->owner);
	}
		break;
	case ev_respawn:
	{
		ev_rp_t *rp;
		rp = (ev_rp_t *) ev->data;
		shRespawn(rp->owner, &rp->newpos, rp->ms, ev->time);
		printf("respawned %d\n", rp->owner);
	}
		break;
	case ev_fire:
	{
		ev_fi_t *fi;

		fi = (ev_fi_t *) ev->data;
		if (!server)
			shFire(fi->owner, &fi->p, fi->weNum, fi->weId, ev->time);
	}
		break;
	case ev_hit:
	{
		ev_hi_t *hi;
		hi = (ev_hi_t *) ev->data;
		shHit(hi->owner, hi->target, hi->part, &hi->p, hi->weId, server, ev->time);
	}
		break;
	case ev_turret:
	{
		ev_tu_t *tu;
		tu = (ev_tu_t *) ev->data;
		tuSetMove(tu->owner, tu->direction, ev->time);
	}
		break;
	case ev_collide:
	{
		ev_co_t *co;
		co = (ev_co_t *) ev->data;
		shCollide(co->owner1, co->owner2, &co->p1, &co->p2, ev->time);
	}
		break;
	default:
		break;
	}
}

void evConsumeEvent(float time) {
	ev_t *ev;

	if (list_empty(&act_event))
		return;
	ev = list_first_entry(&act_event, ev_t, list);
	while (ev && ev->time <= time) {
		evDoEvent(ev, 0);
		list_del(&ev->list);
		list_add(&ev->list, &old_event);
		if (list_empty(&act_event))
			return;
		ev = list_first_entry(&act_event, ev_t, list);
	}
}
