/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <GL/glut.h>
#include "event.h"
#include "ship.h"
#include "ai.h"

LIST_HEAD(old_event);
LIST_HEAD(act_event);

void *evBuff;


void evInit(void) {
	evBuff  = malloc(4096);
}

void evPostEventNow(void *data, event_e type) {
	float time;
	time = glutGet(GLUT_ELAPSED_TIME);
	evPostEvent(time, data, type);
}

void evPostEvent(float time, void *data, event_e type) {
	ev_t *new;
	ev_t *prec;
	new = malloc(sizeof(*new));
	new->type = type;
	new->time = time;

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
		aiCreate(shCreateShip("v2", 1000, 0, 0, 1, 0));
		break;
	case ev_newtraj:
		aiCreate(shCreateShip("mother1", 10000, 0, 0, 1, 0));
		break;
	}
}

void evConsumeEvent(int dummy) {
	ev_t *ev;
	ev_t *safe;
	float cur_time;

	cur_time = glutGet(GLUT_ELAPSED_TIME);
	list_for_each_entry_safe(ev, safe, &act_event, list) {
		if (ev->time > cur_time)
			return;
		evDoEvent(ev);
		list_del(&ev->list);
		list_add(&ev->list, &old_event);
	}
}
