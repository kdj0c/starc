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

LIST_HEAD(server_queue);
LIST_HEAD(server_ships);

int curid = 0xf1457;

int seNewId(void) {
	return curid += 2 * MAX_WEID;
}


// Post event to server
void sePostEventToServer(float time, void *data, int size, event_e type) {
	ev_t *new;
	ev_t *prec;
	new = malloc(sizeof(*new) + size);
	new->type = type;
	new->time = time;
	new->size = size;
	if (size && data)
		memcpy(new->data, data, size);

	if (list_empty(&server_queue)) {
		list_add(&new->list, &server_queue);
		return;
	}
	list_for_each_entry(prec, &server_queue, list) {
		if (prec->time > time)
			break;
	}
	list_add_tail(&new->list, &prec->list);
}

void seStartServerLocal(void) {
	shSetList(&server_ships);
	make_pos(player, 0., 0., 0.);
	make_pos(ai1, -2000., 0., 0.);
	make_pos(ai2, 0., 2000., 0.);
	make_pos(ai3, 0., -2000., 0.);
	make_pos(ai4, 20000., 0., M_PI);
	make_pos(ai5, 15000., 0., 0.);
	make_pos(ai6, 15000., 2000., 0.);
	make_pos(ai7, 15000., -2000., 0.);

	evPostCreateShip("Red5", &pos_player, 0, seNewId(), pl_remote);
	evPostCreateShip("stationRed1", &pos_ai1, 0, seNewId(), pl_ai);
	evPostCreateShip("Red1", &pos_ai2, 0, seNewId(), pl_ai);
	evPostCreateShip("Red3", &pos_ai3, 0, seNewId(), pl_ai);
	evPostCreateShip("stationRed1", &pos_ai4, 1, seNewId(), pl_ai);
	evPostCreateShip("Green2", &pos_ai5, 1, seNewId(), pl_ai);
	evPostCreateShip("Green1", &pos_ai6, 1, seNewId(), pl_ai);
	evPostCreateShip("Green2", &pos_ai7, 1, seNewId(), pl_ai);
}

void seConsumeEvent(float time) {
	ev_t *ev;

	if (list_empty(&server_queue))
		return;
	ev = list_first_entry(&server_queue, ev_t, list);
	while (ev && ev->time <= time) {
		evDoEvent(ev, 1);
		list_del(&ev->list);
		if (list_empty(&server_queue))
			return;
		ev = list_first_entry(&server_queue, ev_t, list);
	}
}


void serverMain(float time) {
	shSetList(&server_ships);

	aiThinkAll(time);
	seConsumeEvent(time);
	shUpdateShips(time);
	seConsumeEvent(time);
	shDetectCollision(time);
	seConsumeEvent(time);
	weUpdate(time);
	seConsumeEvent(time);
}

