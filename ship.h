/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#ifndef SHIP_H_
#define SHIP_H_

#include <GL/glut.h>

typedef struct {
	float x;
	float y;
	float r;
	unsigned int color;
} laser_t;

typedef struct {
	char name[16];
	char imgfile[32];
	GLuint tex;
	float size;
	char shieldfile[32];
	GLuint shieldtex;
	float shieldsize;
	float maxhealth;
	float maniability;
	float thrust;
	int numlaser;
	laser_t laser[5];
} shiptype_t;

typedef struct {
	int direction;
	char acceleration;
	char strafe;
	char fire1;
	char fire2;
} shin_t;

typedef struct ship_s {
	struct ship_s * next;
	float x;
	float y;
	float r;
	float dx;
	float dy;
	float health;
	int team;
	float drawshield;
	float lasnrj;
	shiptype_t * t;
	shin_t in;
} ship_t;

void shDrawShips(void);
void shUpdateShips(float dt);
ship_t * shCreateShip(char *name, float x, float y, float r, int team);
void shLoadShip(void);
ship_t * shFindNearestEnemy(ship_t * self);


#endif /* SHIP_H_ */
