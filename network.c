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
#include <grapple/grapple.h>

#include "config.h"
#include "ship.h"
#include "network.h"

static grapple_client client = 0;
static int clid = 0;

void ntInit(void) {
	ntconf_t ntconf;
	cfReadNetwork(&ntconf);
	client = grapple_client_init("starc", "0.5");
	grapple_client_address_set(client, ntconf.ip);
	grapple_client_port_set(client, ntconf.port);
	grapple_client_protocol_set(client, GRAPPLE_PROTOCOL_UDP);
	grapple_client_start(client, 0);
	grapple_client_name_set(client, ntconf.name);
}

ship_t * ntCreateLocalPlayer(char * type) {
	ship_t * sh;
/*	ntmsg_t *msg;
	static unsigned int count = 0;
	size_t size;

	size = sizeof(ntmsg_t) + sizeof(shipcorename_t);
	msg = malloc(size);
	msg->type = ntSpawn;
	memcpy(msg->NT_SPAWN.ship, sh, sizeof(shipcore_t));
	strcpy(msg->NT_SPAWN.ship[0].typename, type);
	grapple_client_send(client, GRAPPLE_SERVER, GRAPPLE_RELIABLE, msg, size);
	count++; */
	return sh;
}

void ntSendInput(ship_t * sh) {
	char buf[sizeof(shin_t) + sizeof(ntmsg_t) + sizeof(int)];
	ntmsg_t *msg = (ntmsg_t *) buf;
	int size = sizeof(buf);

	msg->type = ntInputs;
	memcpy(&msg->NT_INPUT.in,&sh->in,sizeof(shin_t));
	msg->NT_INPUT.netid = sh->netid;
	grapple_client_send(client, GRAPPLE_SERVER, GRAPPLE_RELIABLE, msg, size);
}

void ntHandleUserMessage(void * data, int size, grapple_user id) {
	ntmsg_t * p;
	int s;
	int local;

	if (!size)
		return;
	p = data;
	switch (p->type) {
	case ntUpdate:
/*		shc = p->NT_UPDATE.ships;
		for (s = sizeof(ntmsg_t); s < size; s += sizeof(shipcore_t)) {
			if (shc->netid >> 8 == clid)
				local = 1;
			else
				local = 0;
//			shSync(shc, local);
			shc++;
		}*/
		break;
	case ntShips:
/*		shn = p->NT_SPAWN.ship;
		for (s = sizeof(ntmsg_t); s < size; s += sizeof(shipcorename_t)) {
			printf("create remote ship %d size %d\n",shn->netid, size);
			shCreateRemoteShip(shn);
			shn++;
		}*/
		break;
	default:
		printf("unexpected message received %d, size %d, id %d\n",p->type, size, id);
		break;
	}
}

void ntHandleMessage(void) {
	grapple_message *message;

	while (grapple_client_messages_waiting(client)) {
		message = grapple_client_message_pull(client);

		switch (message->type) {
		case GRAPPLE_MSG_NEW_USER:
			//Your code to handle this message
			break;
		case GRAPPLE_MSG_NEW_USER_ME:
			//Your code to handle this message
			clid = message->NEW_USER.id;
			printf("my user id is %d\n",message->NEW_USER.id);
			break;
		case GRAPPLE_MSG_USER_NAME:
			//Your code to handle this message
			break;
		case GRAPPLE_MSG_SESSION_NAME:
			//Your code to handle this message
			break;
		case GRAPPLE_MSG_USER_MSG:
			ntHandleUserMessage(message->USER_MSG.data,
					message->USER_MSG.length,
					message->USER_MSG.id);
			break;
		case GRAPPLE_MSG_USER_DISCONNECTED:
			//Your code to handle this message
			break;
		case GRAPPLE_MSG_SERVER_DISCONNECTED:
			//Your code to handle this message
			break;
		case GRAPPLE_MSG_CONNECTION_REFUSED:
			//Your code to handle this message
			break;
		case GRAPPLE_MSG_PING:
			//Your code to handle this message
			break;
		default:
			printf("message not handled %d", message->type);
		}
		grapple_message_dispose(message);
	}
}
