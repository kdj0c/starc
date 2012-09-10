/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef NETWORK_H_
#define NETWORK_H_

#include "event.h"


#if !defined __GNUC__ || (__GNUC__ > 2)
#define BEG_UNION	union {
#define END_UNION   };
#else
#define BUG_UNION
#define END_UNION
#endif

#if defined __GNUC__ && (__GNUC__ < 3)
#define ST_PREFIX(a) _a
#else
#define ST_PREFIX(a)
#endif

#define NEW_UNION(a, b) \
BEG_UNION \
struct \
ST_PREFIX(a) \
{ \
b \
} a; \
END_UNION

typedef struct {
	int type;
    float time;
    NEW_UNION(DATA, char data[4];)
    NEW_UNION(NEWSHIP, ev_cr_t cr;)
    NEW_UNION(NEWTRAJ, ev_tr_t tr;)
    NEW_UNION(RESPAWN, ev_rp_t rp;)
    NEW_UNION(DESTROY, ev_ds_t ds;)
    NEW_UNION(LASER, ev_la_t la;)
    NEW_UNION(TURRET, ev_tu_t tu;)
    NEW_UNION(COLLIDE, ev_co_t co;)
} ntmsg_t;

void ntInit(void);
int ntGetId(void);
void ntHandleMessage(void);
void ntSendEvent(float time, void *data, int size, event_e type);

#endif /* NETWORK_H_ */
