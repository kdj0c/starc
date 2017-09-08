/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef EVENT_H_
#define EVENT_H_

#include "list.h"
#include "vec.h"
#include "ship.h"

typedef enum {
	ev_newship,	//server->client
	ev_newtraj,	//client->server
	ev_respawn,	//server->client
	ev_destroyed,	//server->client
	ev_fire,	//client->server
	ev_hit,
	ev_turret,	//client->server
	ev_collide,	//server->client
	ev_ping	// network only
} event_e;

enum {
	pl_local,
	pl_remote,
	pl_ai
};

typedef struct {
	struct list_head list;
	struct list_head active;
	event_e type;
	float time;
	int size;
	char data[];
} ev_t;

typedef struct {
	int owner;
	int control;
	int team;
	pos_t pos;
	char shipname[16];
} ev_cr_t;

typedef struct {
	int owner;
	shin_t in;
} ev_tr_t;

typedef struct {
	int owner;
	int ms;
	pos_t newpos;
} ev_rp_t;

typedef struct {
	int owner;
} ev_ds_t;

typedef struct {
	int owner;
	pos_t p;
	int id;
} ev_fi_t;

typedef struct {
	int owner;
	int target;
	int part;
	pos_t p;
	int id;
} ev_hi_t;

typedef struct {
	int owner;
	float direction[MAX_TURRET];
} ev_tu_t;

typedef struct {
	int owner1;
	int owner2;
	pos_t p1;
	pos_t p2;
} ev_co_t;

void evConsumeEvent(float time);
void evPostEvent(float time, void *data, int size, event_e type);
void evPostEventLocal(float time, void *data, int size, event_e type);
void evPostEventNow(void *data, int size, event_e type);
void evPostTrajEv(shin_t *in, int owner);
void evPostCreateShip(char *name, pos_t *p, int team, int netid, int control);
void evPostRespawn(pos_t *newp, int netid, int msid, float time);
void evPostDestroy(int netid, float time);
void evPostFire(int owner, pos_t *p, int id, float time);
void evPostHit(int owner, int target, int turret, pos_t *p, int id, float time);
void evPostTurret(int owner, float *dir, float time);
void evPostCollide(int owner1, int owner2, pos_t *p1, pos_t *p2, float time);

#endif
