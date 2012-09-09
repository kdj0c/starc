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

typedef struct {
	int type;
#if !defined __GNUC__ || (__GNUC__ > 2)
	union {
#endif
		struct
#if defined __GNUC__ && (__GNUC__ < 3)
		_EV_T
#endif
		{
			ev_t ev;
		} EV_T;

#if !defined __GNUC__ || (__GNUC__ > 2)
	};
#endif
} ntmsg_t;

void ntInit(void);
void ntHandleMessage(void);
void ntSendEvent(float time, void *data, int size, event_e type);

#endif /* NETWORK_H_ */
