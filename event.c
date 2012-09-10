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

extern int g_net;

LIST_HEAD(old_event);
LIST_HEAD(act_event);

void evPostTrajEv(shin_t *in, int owner) {
	ev_tr_t ev;

	ev.owner = owner;
	ev.in = *in;
	evPostEventNow((void *) &ev, sizeof(ev), ev_newtraj);
}

void evPostDestroy(int netid, float time) {
    ev_ds_t ev;

    ev.owner = netid;
    evPostEvent(time, (void *) &ev, sizeof(ev), ev_destroyed);
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

	ev.owner = netid;
	ev.control = control;
	ev.pos = *p;
	ev.team = team;
	strcpy(ev.shipname, name);
	evPostEventNow((void *) &ev, sizeof(ev), ev_newship);
}

void evPostLaser(int owner, pos_t *p, unsigned int color, float lifetime, float len, float width, int id, float time) {
    ev_la_t ev;

    ev.owner = owner;
    ev.width = width;
    ev.color = color;
    ev.len = len;
    ev.p = *p;
    ev.id = id;
    ev.lifetime = lifetime;
    evPostEvent(time, (void *) &ev, sizeof(ev), ev_laser);
}

void evPostHit(int owner, int target, pos_t *p, int id, float time) {
    ev_hi_t ev;

    ev.owner = owner;
    ev.target = target;
    ev.p = *p;
    ev.id = id;
    printf("hit %d\n", id);
    evPostEvent(time, (void *) &ev, sizeof(ev), ev_hit);
}

void evPostTurret(int owner, signed char *dir, float time) {
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
	if (size && data)
		memcpy(&new->data, data, size);

	if (list_empty(&act_event)) {
		list_add(&new->list, &act_event);
		return;
	}
	list_for_each_entry(prec, &act_event, list) {
		if (prec->time > time)
			break;
	}
	list_add_tail(&new->list , &prec->list);
}

void evPostEvent(float time, void *data, int size, event_e type) {
    evPostEventLocal(time, data, size, type);
    ntSendEvent(time, data, size, type);
}

void evDoEvent(ev_t *ev) {
//    printf("do event %d, %f\n", ev->type, ev->time);
	switch (ev->type) {
	case ev_newship:
	{
		ev_cr_t *cr;
		ship_t *sh;
		cr = (ev_cr_t *) ev->data;
		sh = shCreateShip(cr->shipname, &cr->pos, cr->team, cr->owner, ev->time);
		shLoadShip();
		if (cr->control == pl_ai)
			aiCreate(sh);
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
            shDestroy(ds->owner, ev->time);
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
	case ev_laser:
        {
            ev_la_t *la;
            la = (ev_la_t *) ev->data;
            shLaser(la->owner, &la->p, la->len, la->width, la->lifetime, la->color, la->id, ev->time);
        }
        break;
	case ev_hit:
        {
            ev_hi_t *hi;
            hi = (ev_hi_t *) ev->data;
            shHit(hi->owner, hi->target, &hi->p, hi->id, ev->time);
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
	}
}

void evConsumeEvent(float time) {
	ev_t *ev;

    if (list_empty(&act_event))
        return;
	ev = list_first_entry(&act_event, ev_t, list);
    while (ev && ev->time <= time) {
        evDoEvent(ev);
        list_del(&ev->list);
        list_add(&ev->list, &old_event);
        if (list_empty(&act_event))
            return;
        ev = list_first_entry(&act_event, ev_t, list);
    }
}
