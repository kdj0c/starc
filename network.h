/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef NETWORK_H_
#define NETWORK_H_

enum {
    e_event,
};

typedef struct {
	int type;
#if !defined __GNUC__ || (__GNUC__ > 2)
	union {
#endif
		struct
#if defined __GNUC__ && (__GNUC__ < 3)
		_NT_SPAWN
#endif
		{
			int ship[0];
		} NT_SPAWN;
#if defined __GNUC__ && (__GNUC__ < 3)
		_NT_UPDATE
#endif
		struct {
			int ships[0];
		} NT_UPDATE;
		struct
#if defined __GNUC__ && (__GNUC__ < 3)
		_NT_INPUT
#endif
		{
			shin_t in;
			int netid;
		} NT_INPUT;
#if !defined __GNUC__ || (__GNUC__ > 2)
	};
#endif
} ntmsg_t;

void ntInit(void);
ship_t * ntCreateLocalPlayer(char * type);
void ntHandleMessage(void);
void ntSendInput(ship_t * sh);


#endif /* NETWORK_H_ */
