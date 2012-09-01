/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <GL/glut.h>
#include <string.h>
#include <stdio.h>

#include "event.h"
#include "ship.h"
#include "ai.h"
#include "weapon.h"

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

void evPostCreateShip(char *name, pos_t *p, int team, int netid) {
	ev_cr_t ev;

	ev.owner = netid;
	ev.pos = *p;
	ev.team = team;
	strcpy(ev.shipname, name);
	evPostEventNow((void *) &ev, sizeof(ev), ev_newship);
}

void evPostLaser(int owner, pos_t *p, unsigned int color, float lifetime, float len, float width, float time) {
    ev_la_t ev;

    ev.owner = owner;
    ev.width = width;
    ev.color = color;
    ev.len = len;
    ev.p = *p;
    ev.lifetime = lifetime;
    evPostEvent(time, (void *) &ev, sizeof(ev), ev_laser);
}

void evPostEventNow(void *data, int size, event_e type) {
	float time;
	time = glutGet(GLUT_ELAPSED_TIME);
	evPostEvent(time, data, size, type);
}

void evPostEvent(float time, void *data, int size, event_e type) {
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

void evDoEvent(ev_t *ev) {
	switch (ev->type) {
	case ev_newship:
	{
		ev_cr_t *cr;
		ship_t *sh;
		cr = (ev_cr_t *) ev->data;
		sh = shCreateShip(cr->shipname, &cr->pos, cr->team, cr->owner);
		shLoadShip();
		if (cr->owner > 0)
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
            shDestroy(ds->owner);
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
            shLaser(la->owner, &la->p, la->len, la->width, la->lifetime, la->color, ev->time);
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
