/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "ship.h"

typedef struct {
	long fullscreen;
	long width;
	long heigh;
} grconf_t;

typedef struct {
	char ip[32];
	int port;
	char name[64];
} ntconf_t;

void cfReadGraphic(grconf_t * c);
void cfReadNetwork(ntconf_t *c);

int cfReadGameData(void);

shiptype_t * cfGetShip(const char * name);
turrettype_t * cfGetTurret(const char * name);

#endif /* CONFIG_H_ */
