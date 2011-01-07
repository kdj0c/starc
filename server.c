/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <unistd.h>
#include <stdio.h>
#include <grapple.h>

int main(int argc, char *argv[]) {
	grapple_server server;
	grapple_message * message;
	int test = 0;

	server = grapple_server_init("starc", "0.4");
	grapple_server_ip_set(server, "127.0.0.1"); //OPTIONAL FUNCTION
	grapple_server_port_set(server, 1234);
	grapple_server_protocol_set(server, GRAPPLE_PROTOCOL_UDP);
	grapple_server_session_set(server, "Play my game");
	grapple_server_start(server);

	while (1) {
		test = grapple_server_intvar_get(server, "test_variable");
//		if (test)
			printf("test is %d\n", test);
		sleep(1);
		if (grapple_server_messages_waiting(server)) {
			message = grapple_server_message_pull(server);

			switch (message->type) {
			case GRAPPLE_MSG_NEW_USER:
				printf("new user\n");
				break;
			case GRAPPLE_MSG_USER_NAME:
				printf("new user name\n");
				//Your code to handle this message
				break;
			case GRAPPLE_MSG_USER_MSG:
				printf("new user msg\n");
				//Your code to handle this message
				break;
			case GRAPPLE_MSG_USER_DISCONNECTED:
				printf("user disconnected\n");
				//Your code to handle this message
				break;
			}
			grapple_message_dispose(message);
		}
	}
}
