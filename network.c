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
#include "event.h"

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

void ntHandleUserMessage(void *data, int size, grapple_user id) {
	ntmsg_t *p;

	if (!size)
		return;
	p = data;
    evPostEventLocal(p->time, p->DATA.data, size, p->type);
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

void ntSendEvent(float time, void *data, int size, event_e type) {
    char buf[4096];
    ntmsg_t *msg = (ntmsg_t *) buf;

    msg->type = type;
    msg->time = time;
    memcpy(msg->DATA.data, data, size);

    if (type == ev_newship) {
        ev_cr_t *cr;
        cr = (ev_cr_t *) msg->DATA.data;
        cr->control = pl_remote;
    }
    grapple_client_send(client, GRAPPLE_EVERYONE, GRAPPLE_RELIABLE, msg, size + sizeof(ntmsg_t) + sizeof(int));
}
