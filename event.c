/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <GL/glut.h>
#include <string.h>
#include "event.h"
#include "ship.h"
#include "ai.h"

LIST_HEAD(old_event);
LIST_HEAD(act_event);

void *evBuff;


void evInit(void) {
	evBuff  = malloc(4096);
}

void evPostTrajEv(shin_t *in, int owner) {
	ev_tr_t ev;

	ev.owner = owner;
	ev.in = *in;
	evPostEventNow((void *) &ev, sizeof(ev), ev_newtraj);
}

void evPostCreateShip(char *name, float x, float y, float r, int team, int netid) {
	ev_cr_t ev;

	ev.owner = netid;
	ev.pos.p.x = x;
	ev.pos.p.y = y;
	ev.pos.r = r;
	ev.team = team;
	strcpy(ev.shipname, name);
	evPostEventNow((void *) &ev, sizeof(ev), ev_newship);
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
	list_add(&new->list , &prec->list);
}

void evDoEvent(ev_t *ev) {
	switch (ev->type) {
	case ev_newship:
	{
		ev_cr_t *cr;
		ship_t *sh;
		cr = (ev_cr_t *) ev->data;
		sh = shCreateShip(cr->shipname, cr->pos.p.x, cr->pos.p.y, cr->pos.r, cr->team, cr->owner);
		shLoadShip();
		if (cr->owner > 0)
			aiCreate(sh);
	}
		break;
	case ev_newtraj:
		{
			ev_tr_t *tr;
			tr = (ev_tr_t *) ev->data;
			shSetInput(&tr->in, tr->owner);
		}
		break;
	}
}

void evConsumeEvent(float time) {
	ev_t *ev;
	ev_t *safe;

	list_for_each_entry_safe(ev, safe, &act_event, list) {
		if (ev->time > time)
			return;
		evDoEvent(ev);
		list_del(&ev->list);
		list_add(&ev->list, &old_event);
	}
}
