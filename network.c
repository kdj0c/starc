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
#include <unistd.h>
#include <grapple/grapple.h>

#include "config.h"
#include "ship.h"
#include "network.h"
#include "event.h"
#include "gametime.h"
#include "ai.h"
#include "weapon.h"

typedef enum {
	e_disconnected,
	e_client,
	e_server
} net_e;

static grapple_client client = 0;
static int clid = 0;
static grapple_server server;
static ntmsg_t *datas;
static net_e status = e_disconnected;
static int curid = 0;
static float synctime;

void ntSendPing(void) {
	ntmsg_t msg;

	synctime = gtGetTime();
	msg.type = ev_ping;
	msg.time = synctime;
	grapple_client_send(client, GRAPPLE_SERVER, GRAPPLE_RELIABLE, &msg, sizeof(ntmsg_t));
}

void ntInit(void) {
	ntconf_t ntconf;

	if (status != e_disconnected)
		return;
	cfReadNetwork(&ntconf);
	client = grapple_client_init("starc", "0.5");
	grapple_client_address_set(client, ntconf.ip);
	grapple_client_port_set(client, ntconf.port);
	grapple_client_protocol_set(client, GRAPPLE_PROTOCOL_UDP);
	grapple_client_start(client, 0);
	grapple_client_name_set(client, ntconf.name);
	ntSendPing();
	while (synctime)
		ntHandleMessage();
	ntSendPing();
	while (synctime)
		ntHandleMessage();
	status = e_client;
}

int ntGetId(void) {
	return curid++;
}

void ntSyncPing(ntmsg_t *p) {
	float rtime;
	float offset;

	rtime = gtGetTime();
	offset = ((rtime + synctime) / 2.f) - p->time;
	synctime = 0.;
	if (status == e_client) {
		//runtime correction, make it smooth !
		offset /= 5.;
		if (offset > 10.)
			offset = 10.;
		else if (offset < -10.)
			offset = -10.;
	}
	gtSetOffset(offset);
	printf("client time %f, server time %f, offset %f\n", rtime, p->time, offset);
	printf("new time %f\n", gtGetTime());
}

void ntHandleUserMessage(void *data, int size, grapple_user id) {
	ntmsg_t *p;

	if (!size)
		return;
	p = data;

	if (p->type == ev_ping) {
		ntSyncPing(p);
		return;
	}
//  printf("client receive new message %d\n", p->type);
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
			curid = 256 * clid;
			printf("my user id is %d\n", message->NEW_USER.id);
			break;
		case GRAPPLE_MSG_USER_NAME:
			//Your code to handle this message
			break;
		case GRAPPLE_MSG_SESSION_NAME:
			//Your code to handle this message
			break;
		case GRAPPLE_MSG_USER_MSG:
			ntHandleUserMessage(message->USER_MSG.data, message->USER_MSG.length, message->USER_MSG.id);
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
			printf("client receive ping\n");
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
	if (status == e_client) {
		grapple_client_send(client, GRAPPLE_SERVER, GRAPPLE_RELIABLE, msg, size + sizeof(ntmsg_t) + sizeof(int));
		grapple_client_send(client, GRAPPLE_EVERYONEELSE, GRAPPLE_RELIABLE, msg, size + sizeof(ntmsg_t) + sizeof(int));
	} else if (status == e_server)
		grapple_server_send(server, GRAPPLE_EVERYONE, GRAPPLE_RELIABLE, msg, size + sizeof(ntmsg_t) + sizeof(int));
}

void svInit(void) {
	ntconf_t ntconf;

	if (status != e_disconnected)
		return;
	cfReadNetwork(&ntconf);
	cfReadGameData();
	weInit();

	datas = malloc(sizeof(ntmsg_t) + 4096);
	server = grapple_server_init("starc", "0.5");

	grapple_server_port_set(server, ntconf.port);
	grapple_server_protocol_set(server, GRAPPLE_PROTOCOL_UDP);
	grapple_server_session_set(server, "Play my game");
	grapple_server_start(server);
	status = e_server;
	printf("server started, port %d\n", ntconf.port);
}

void svSendPing(int client) {
	ntmsg_t msg;
	float time;

	time = gtGetTime();
	msg.type = ev_ping;
	msg.time = time;
	grapple_server_send(server, client, GRAPPLE_RELIABLE, &msg, sizeof(ntmsg_t));
}

void svHandleUserMessage(void *data, int size, grapple_user id) {
	ntmsg_t *p;

	if (!size)
		return;

	p = data;

	if (p->type == ev_ping) {
		svSendPing(id);
		return;
	}
//    printf("host receive new message %d\n", p->type);
	evPostEventLocal(p->time, p->DATA.data, size, p->type);
}

void svNewClient(grapple_message * message) {
	char buf[4096];
	int nships, i;
	float time;
	ev_cr_t *ev;

	ntmsg_t *msg = (ntmsg_t *) datas;

	time = 0.f;
	nships = shPostAllShips(time, buf);
	ev = (ev_cr_t *) buf;
	for (i = 0; i < nships; i++) {
		msg->type = ev_newship;
		msg->time = time;
		memcpy(msg->DATA.data, ev, sizeof(*ev));
		grapple_server_send(server, message->NEW_USER.id, GRAPPLE_RELIABLE, datas, sizeof(*ev) + sizeof(*msg));
		ev++;
	}
}

void svHandleMessage(void) {
	grapple_message *message;

	if (grapple_server_messages_waiting(server)) {
		message = grapple_server_message_pull(server);

		switch (message->type) {
		case GRAPPLE_MSG_NEW_USER:
			svNewClient(message);
			printf("new user %d\n", message->NEW_USER.id);
			break;
		case GRAPPLE_MSG_USER_NAME:
			printf("new user name %s\n", message->USER_NAME.name);
			//Your code to handle this message
			break;
		case GRAPPLE_MSG_USER_MSG:
			svHandleUserMessage(message->USER_MSG.data, message->USER_MSG.length, message->USER_MSG.id);
			break;
		case GRAPPLE_MSG_USER_DISCONNECTED:
			printf("user disconnected\n");
			//Your code to handle this message
			shDisconnect(message->USER_DISCONNECTED.id);
			break;
		case GRAPPLE_MSG_PING:
			printf("server receive ping\n");
			break;

		default:
			printf("unexpected message received\n");
			break;
		}
		grapple_message_dispose(message);
	}
}

#ifdef DEDICATED
void svLoop(void) {
	float time;

	svHandleMessage();
	time = gtGetTime();
	aiThink(time);
	evConsumeEvent(time);
	shUpdateLocal(time);
	shUpdateShips(time);
	shDetectCollision(time);
	weUpdate(time);
	evConsumeEvent(time);
}

int main(int argc, char *argv[]) {
	make_pos(ai1, 0., 5000., 0.);
	make_pos(mother, 0., 10000., 0.);
	make_pos(ai2, 5000., 0., 0.);
	make_pos(ai3, 5000., 3000., 0.);

	gtInit();
	svInit();
	evPostCreateShip("v2", &pos_ai1, 0, ntGetId(), pl_ai);
	evPostCreateShip("mother1", &pos_mother, 0, ntGetId(), pl_ai);

	evPostCreateShip("w1", &pos_ai2, 1, ntGetId(), pl_ai);
	evPostCreateShip("w2", &pos_ai3, 1, ntGetId(), pl_ai);

	while (1) {
		svLoop();
		usleep(10000);
	}
	free(datas);
}
#endif
