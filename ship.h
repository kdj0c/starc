/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#ifndef SHIP_H_
#define SHIP_H_

#define LASER_RANGE 5000

typedef struct {
	float x;
	float y;
	float r;
	unsigned int color;
} laser_t;

typedef struct {
	float x;
	float y;
	unsigned int color;
} burst_t;

typedef struct {
	char name[16];
	char imgfile[32];
	unsigned int tex;
	float size;
	char shieldfile[32];
	unsigned int shieldtex;
	float shieldsize;
	float maxhealth;
	float maniability;
	float thrust;
	int numlaser;
	laser_t laser[5];
	int numburst;
	burst_t burst[5];
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
	int team;			\
	float drawshield;	\
	float lasnrj;		\
	shin_t in;

/*
 * only core, data synchronized by network
 */
typedef struct {
	ship_core
} shipcore_t;

/*
 * append name at the end for network init
 */
typedef struct {
	ship_core
	char typename[16];
} shipcorename_t;

/*
 * local structure
 */
typedef struct ship_s {
	ship_core
	struct ship_s * next;
	shiptype_t * t;
	int netid;
} ship_t;

void shDrawShips(void);
void shUpdateShips(float dt);
ship_t * shCreateShip(char *name, float x, float y, float r, int team);
ship_t * shCreateRemoteShip(shipcorename_t * shn, int netid);
void shSync(shipcore_t * shc, int netid);
void shLoadShip(void);
ship_t * shFindNearestEnemy(ship_t * self);


#endif /* SHIP_H_ */
