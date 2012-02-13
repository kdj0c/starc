/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef EVENT_H_
#define EVENT_H_

typedef enum {
	ev_newship,
	ev_newtraj,
} event_e;

#include "list.h"
typedef struct {
	struct list_head list;
	struct list_head active;
	event_e type;
	float time;
	void *pdata;
} ev_t;


void evConsumeEvent(int dummy);
void evPostEvent(float time, void *data, event_e type);
void evPostEventNow(void *data, event_e type);

#endif
