/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <string.h>
#include <stdio.h>
#include <grapple.h>
#include "ship.h"

static grapple_client client = 0;
extern ship_t * player;

void ntInit(void) {
	client = grapple_client_init("starc", "0.4");
	grapple_client_address_set(client, NULL);
	grapple_client_port_set(client, 1234);
	grapple_client_protocol_set(client, GRAPPLE_PROTOCOL_UDP);
	grapple_client_start(client, 0);
	grapple_client_name_set(client, "joc");
}

ship_t * ntCreateLocalPlayer(char * type) {
	ship_t * sh;
	shipcorename_t * shn;

	sh = shCreateShip(type, 0, 0, 0, 0);
	shn = (shipcorename_t *) sh;
	strcpy(shn->typename, type);
	grapple_client_send(client, GRAPPLE_EVERYONEELSE, GRAPPLE_RELIABLE, shn,
			sizeof(*shn));
	return sh;
}

void ntSendShip(ship_t * sh) {
	shipcore_t * shc;
	shc = (shipcore_t *) sh;

	grapple_client_send(client, GRAPPLE_EVERYONEELSE, 0, shc,
				sizeof(*shc));
}

void ntHandleUserMessage(void * data, int size, grapple_user id) {
	if (size == sizeof(shipcorename_t)) {
		printf("receive new remote ship \n");
		shipcorename_t * shn;
		shn = (shipcorename_t *) data;
		shCreateRemoteShip(shn, id);
	} else if (size == sizeof(shipcore_t)) {
		printf("receive update remote ship \n");
		shipcore_t * shc;
		shc = (shipcore_t *) data;
		shSync(shc,id);
	} else {
		printf("network error \n");
	}
}

void ntHandleMessage(void) {
	grapple_message *message;

	while (grapple_client_messages_waiting(client)) {
		message = grapple_client_message_pull(client);

		switch (message->type) {
		case GRAPPLE_MSG_NEW_USER:
			ntSendShip(player);
			//Your code to handle this message
			break;
		case GRAPPLE_MSG_NEW_USER_ME:
			//Your code to handle this message
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
