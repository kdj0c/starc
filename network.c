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
#include <uv.h>

#include "config.h"
#include "ship.h"
#include "network.h"
#include "event.h"
#include "gametime.h"
#include "ai.h"
#include "weapon.h"

#define STARC_PORT 7834

uv_loop_t *loop;

#define check_uv(status) \
  do { \
      int code = (status); \
      if(code < 0){ \
          fprintf(stderr, "%s: %s\n", uv_err_name(code), uv_strerror(code)); \
          exit(code); \
      } \
  } while(0)

static void ntAllocBuffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
	buf->base = malloc(suggested_size);
	buf->len = suggested_size;
}

void ntOnRead(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
	if (nread < 0)
		printf("Error %d\n", nread);
	else if (nread > 0) {
		printf("NREAD %d ", nread);
		int i;
		for (i = 0; i < nread; i++)
			printf("%c ", (char) *(buf->base + i));
		printf("\n");
	}
}

void ntOnWrite(uv_write_t *req, int status) {
	check_uv(status);

	if (req) {
		if (req->data)
			free(req->data);
		free(req);
	}
}

void ntOnConnect(uv_connect_t *req, int status) {
	uv_write_t *reqw;
	uv_buf_t buf;

	check_uv(status);

	reqw = malloc(sizeof(uv_write_t));

	buf = uv_buf_init(malloc(10), 10);

	memcpy(buf.base, "echo test\0", 10);

	uv_read_start(req->handle, ntAllocBuffer, ntOnRead);
	check_uv(uv_write(reqw, req->handle, &buf, 1, ntOnWrite));	// Write to the TCP socekt
}

void ntOnNewConnection(uv_stream_t *server, int status) {
	check_uv(status);
	uv_tcp_t *client = (uv_tcp_t *) malloc(sizeof(uv_tcp_t));
	uv_tcp_init(loop, client);

	if (uv_accept(server, (uv_stream_t *) client) == 0) {
		uv_read_start((uv_stream_t *) client, ntAllocBuffer, ntOnRead);
	} else {
		uv_close((uv_handle_t *) client, NULL);
	}
}

int ntServerInit(void) {
	struct sockaddr_in addr;
	loop = uv_default_loop();

	uv_tcp_t server;
	uv_tcp_init(loop, &server);
	uv_tcp_nodelay(&server, 1);

	uv_ip4_addr("0.0.0.0", STARC_PORT, &addr);

	uv_tcp_bind(&server, (const struct sockaddr *) &addr, 0);
	check_uv(uv_listen((uv_stream_t *) & server, 2, ntOnNewConnection));

	//uv_run(loop, UV_RUN_DEFAULT);

	return 0;
}

int ntClientInit(void) {
	struct sockaddr_in dest;

	loop = uv_default_loop();

	uv_tcp_t *socket = (uv_tcp_t *) malloc(sizeof(uv_tcp_t));
	uv_tcp_init(loop, socket);
	uv_tcp_nodelay(socket, 1);

	uv_connect_t *connect = (uv_connect_t *) malloc(sizeof(uv_connect_t));

	uv_ip4_addr("127.0.0.1", STARC_PORT, &dest);

	uv_tcp_connect(connect, socket, (const struct sockaddr *) &dest, ntOnConnect);

	uv_run(loop, UV_RUN_DEFAULT);

	return 0;
}

void ntSendPing(void) {
}
void ntSendEvent(float time, void *data, int size, event_e type) {
}
void ntHandleMessage(void) {
}

#ifdef DEDICATED
void svLoop(void) {
	float time;

	svHandleMessage();
	time = gtGetTime();
	aiThinkAll(time);
	evConsumeEvent(time);
	shUpdateLocal(time);
	shUpdateShips(time);
	shDetectCollision(time);
	weUpdate(time);
	evConsumeEvent(time);
}

void ntServerMain(void) {
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
}
#endif
