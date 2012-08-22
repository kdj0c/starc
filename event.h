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

typedef enum {
	ev_newship,
	ev_newtraj,
} event_e;

enum {
	local_player,
	remote_player,
	ai_player
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
	pos_t pos;
	char shipname[16];
} ev_cr_t;

void evConsumeEvent(int dummy);
void evPostEvent(float time, void *data, int size, event_e type);
void evPostEventNow(void *data, int size, event_e type);
void evPostCreateShip(int owner, pos_t pos, char * name);

#endif
