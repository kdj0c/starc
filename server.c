/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <grapple/grapple.h>

#include "ship.h"
#include "ai.h"
#include "network.h"
#include "config.h"

static grapple_server server;
static ntmsg_t *datas;

void svHandleUserMessage(void * data, int size, grapple_user id) {
	ntmsg_t * p;
	ntmsg_t * new;
	shipcorename_t * shn;
	int s;

	if(!size)
		return;

	p = data;
	switch (p->type) {
	case ntSpawn:
		shn = p->NT_SPAWN.ship;
		for (s = sizeof(ntmsg_t); s < size; s += sizeof(shipcorename_t)) {
			printf("create ship on the server id %d\n", shn->netid);
			shCreateRemoteShip(shn);
			shn++;
		}
		new = malloc(size);
		memcpy(new, p, size);
		new->type = ntShips;
		grapple_server_send(server, GRAPPLE_EVERYONE, GRAPPLE_RELIABLE, new,
				size);
		free(new);
		break;
	case ntInputs:
		shSetInput(&p->NT_INPUT.in, p->NT_INPUT.netid);
		break;
	default:
		printf("unexpected user message received\n");
		break;
	}
}

void loop() {
	grapple_message * message;
	int size;
	if (grapple_server_messages_waiting(server)) {
		message = grapple_server_message_pull(server);

		switch (message->type) {
		case GRAPPLE_MSG_NEW_USER:
			size = shSerializeOnce(datas->NT_SPAWN.ship) + sizeof(ntmsg_t);
			datas->type = ntShips;
			grapple_server_send(server, message->NEW_USER.id, GRAPPLE_RELIABLE,
					datas, size);
			printf("new user\n");
			break;
		case GRAPPLE_MSG_USER_NAME:
			printf("new user name\n");
			//Your code to handle this message
			break;
		case GRAPPLE_MSG_USER_MSG:
			svHandleUserMessage(message->USER_MSG.data,
					message->USER_MSG.length,
					message->USER_MSG.id);
			break;
		case GRAPPLE_MSG_USER_DISCONNECTED:
			printf("user disconnected\n");
			//Your code to handle this message
			shDisconnect(message->USER_DISCONNECTED.id);
			break;
		default:
			printf("unexpected message received\n");
			break;
		}
		grapple_message_dispose(message);
	}

	aiThink();
	shUpdateShips(50);
	shUpdateRespawn(50);
	shDetectCollision();
	size = shSerialize(datas->NT_UPDATE.ships);
	size += sizeof(ntmsg_t);
	datas->type = ntUpdate;
	grapple_server_send(server, GRAPPLE_EVERYONE, 0, datas, size);
}

int main(int argc, char *argv[]) {
	ntconf_t ntconf;
	cfReadNetwork(&ntconf);
	datas = malloc(sizeof(ntmsg_t) + sizeof(shipcorename_t) * 50);
	server = grapple_server_init("starc", "0.4");

	grapple_server_port_set(server, ntconf.port);
	grapple_server_protocol_set(server, GRAPPLE_PROTOCOL_UDP);
	grapple_server_session_set(server, "Play my game");
	grapple_server_start(server);

	shLoadShipType();
	aiCreate(shCreateShip("v1", 10000, 0, -1, 1, 0));

	while (1) {
		loop();
		usleep(50000);
	}
	free(datas);
}
