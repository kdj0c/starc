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
#include "graphic.h"
#include "weapon.h"

#define LASER_RANGE 15000

/* Ship Flags */
#define	SH_MOTHERSHIP 0x1

#define MAX_TURRET 10
#define MAX_WEAPON 5
#define MAX_ENGINE 5
#define MAX_PARTS 50

typedef struct {
	vec_t p;
	float r;
	weapontype_t *wt;
} weapon_t;

typedef struct {
	vec_t p;
	float size;
	unsigned int color;
} burst_t;

typedef struct {
	char name[16];
	char imgfile[128];
	texc_t tex;
	float size;
	texc_t shieldtex;
	unsigned int shieldcolor;
	float shieldsize;
	float maxhealth;
	float maniability;
	unsigned int flag;
	int numweapon;
	weapon_t laser[MAX_WEAPON];
} turrettype_t;

typedef struct {
	int i;		// index of station part the turret is attached to
	turrettype_t *t;
} turretpos_t;

typedef struct {
	vec_t p;
	float r;
} hangar_t;

typedef struct {
	char name[16];
	unsigned int cmask;
	texc_t tex;
	float maxhealth;
} parttype_t;

typedef struct {
	vec_t p;
	float r;
	parttype_t *part;
} part_t;

typedef struct {
	char name[16];
	texc_t texture;
	/* diameter */
	float size;
	char shieldfile[32];
	texc_t shieldtexture;
	unsigned int shieldcolor;
	float shieldsize;
	float maxhealth;
	float maniability;
	float thrust;
	unsigned int flag;
	int numweapon;
	weapon_t laser[MAX_WEAPON];
	int numburst;
	burst_t burst[MAX_ENGINE];
	int numturret;
	turretpos_t turret[MAX_TURRET];
	hangar_t hangar;
	int numparts;
	part_t part[MAX_PARTS];
} shiptype_t;

typedef struct {
	float direction;
	float acceleration;
	char fire1;
	char fire2;
} shin_t;

struct tur;
/*
 * local structure
 */
typedef struct ship_s {
	struct list_head list;
	shiptype_t *t;
	struct tur *turret;
	float respawn_time;
	float health;
	float lastdamage;
	float engtime;
	float thrusttime;
	float hgRespawn;	// for mothership to avoid all ships respawning at the same time.
	int netid;
	int team;
	shin_t in;
	traj_t traj;
	pos_t pos;
	float lastfire[MAX_WEAPON];
} ship_t;

struct tur {
	vec_t p;
	float r;
	float basetime;
	float baseaim;
	ship_t *target;
	float lastthink;
	float lastfire[MAX_WEAPON];
	float lastdamage;
	float dir;
	float health;
};

typedef struct tur turret_t;

void shDrawShips(float time);
void shDrawShields(float time);
ship_t *shGetByID(int id);
ship_t *shGetPlayer(void);
void shSetPlayer(ship_t *sh);
void shLoadShipType(void);
void shUpdateLocal(float time);
void shUpdateShips(float time);
void shFire(int netid, pos_t *p, int id, float time);
void shFireWeapon(ship_t *sh, pos_t *p, int l, float time);
void shHit(int owner, int tgid, int turret, pos_t *p, int weid, float time);
void shDetectCollision(float time);
int shDetectHit(int netid, pos_t *p, float size, int weid, float time);
void shDamage(ship_t *sh, float dg, float time);
void shRespawn(int netid, pos_t *np, int msid, float time);
void shDestroy(int netid, float time);
ship_t *shCreateShip(char *name, pos_t *pos, int team, int netid, float time);
ship_t *shFindMotherShip(int team);
ship_t *shFindNearestEnemy(ship_t *self);
void shNewTraj(shin_t *in, int netid, float time);
void shDisconnect(int clid);
int shPostAllShips(float time, void *data);
void shCollide(int netid1, int netid2, pos_t *p1, pos_t *p2, float time);
#endif /* SHIP_H_ */
