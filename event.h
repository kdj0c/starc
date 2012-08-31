/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef EVENT_H_
#define EVENT_H_


#include "list.h"
#include "vec.h"

#include "ship.h"

typedef enum {
	ev_newship,
	ev_newtraj,
} event_e;

enum {
	pl_local,
	pl_remote,
	pl_ai
};

typedef struct {
	struct list_head list;
	struct list_head active;
	event_e type;
	float time;
	char data[];
} ev_t;

typedef struct {
	int owner;
	int team;
	pos_t pos;
	char shipname[16];
} ev_cr_t;

typedef struct {
	int owner;
	shin_t in;
} ev_tr_t;

void evConsumeEvent(float time);
void evPostEvent(float time, void *data, int size, event_e type);
void evPostEventNow(void *data, int size, event_e type);
void evPostTrajEv(shin_t *in, int owner);
void evPostCreateShip(char *name, pos_t *p, int team, int netid);

#endif
