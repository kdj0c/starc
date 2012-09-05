/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#ifndef SHIP_H_
#define SHIP_H_

#include "list.h"
#include "vec.h"

#define LASER_RANGE 5000

/* Ship Flags */
#define	SH_MOTHERSHIP 0x1

#define MAX_TURRET 10
#define MAX_LASER 5
#define MAX_ENGINE 5


typedef struct {
    vec_t p;
	float r;
	unsigned int color;
} laser_t;

typedef struct {
    vec_t p;
	float size;
	unsigned int color;
} burst_t;

typedef struct {
	char name[16];
	char imgfile[128];
	unsigned int tex;
	float size;
	char shieldfile[32];
	unsigned int shieldtex;
	float shieldsize;
	float maxhealth;
	float maniability;
	unsigned int flag;
	int numlaser;
	laser_t laser[MAX_LASER];
} turrettype_t;

typedef struct {
    vec_t p;
	turrettype_t * t;
} turretpos_t;

typedef struct {
	vec_t p;
	float r;
} hangar_t;

typedef struct {
	char name[16];
	char imgfile[128];
	unsigned int tex;
	float size;
	char shieldfile[32];
	unsigned int shieldtex;
	float shieldsize;
	float maxhealth;
	float maniability;
	float thrust;
	unsigned int flag;
	int numlaser;
	laser_t laser[MAX_LASER];
	int numburst;
	burst_t burst[MAX_ENGINE];
	int numturret;
	turretpos_t turret[MAX_TURRET];
	hangar_t hangar;
} shiptype_t;

typedef struct {
	int direction;
	char acceleration;
	char strafe;
	char fire1;
	char fire2;
} shin_t;

/*
 * core component of ship type
 */
#define ship_core		\
	float x;			\
	float y;			\
	float r;			\
	float dx;			\
	float dy;			\
	float health;		\
	float drawshield;	\
	int netid;			\
	int team;			\
	shin_t in;          \
	traj_t traj;        \
	pos_t pos;

/*
 * only core, data synchronized by network
 */
typedef struct {
	ship_core
} shipcore_t;

/*
 * add name before for network init
 */
typedef struct {
	char typename[16];
	ship_core
} shipcorename_t;

struct tur;
/*
 * local structure
 */
typedef struct ship_s {
	struct list_head list;
	shiptype_t * t;
	struct tur * turret;
	float respawn_time;
	float health;
	float lastdamage;
	float engtime;
	int netid;
	int team;
	shin_t in;
	traj_t traj;
	pos_t pos;
	float lastfire;
} ship_t;

struct tur {
    vec_t p;
	float r;
	float basetime;
	float baseaim;
	ship_t *target;
	float lastthink;
	float lastfire;
	float lastdamage;
	signed char dir;
	float health;
};

typedef struct tur turret_t;

#ifndef DEDICATED
void shDrawShips(void);
void shLoadShip(void);
void shDrawShipHUD(ship_t * pl);
#endif
ship_t *shGetByID(int id);
void shLoadShipType(void);
void shUpdateShips(float dt);
void shLaser(int netid, pos_t *p, float len, float width, float lifetime, unsigned int color, float time);
void shFireLaser(ship_t *sh, pos_t *p, float time);
void shDetectCollision(void);
void shUpdateRespawn(float dt);
void shDamage(ship_t *sh, float dg, float time);
void shRespawn(int netid, pos_t *np, int msid, float time);
void shDestroy(int netid);
ship_t * shCreateShip(char *name, pos_t *pos, int team, int netid);
ship_t * shCreateRemoteShip(shipcorename_t * shn);
void shSync(shipcore_t * shc, int local);
ship_t * shFindMotherShip(int team);
ship_t * shFindNearestEnemy(ship_t * self);
int shSerialize(shipcore_t * data);
int shSerializeOnce(shipcorename_t * data);
void shSetInput(shin_t * in, int netid);
void shNewTraj(shin_t *in, int netid,  float time);
void shDisconnect(int clid);

#endif /* SHIP_H_ */
