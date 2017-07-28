/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __CONFIG_H_
#define __CONFIG_H_

#include "ship.h"
#include "graphic.h"

typedef struct {
	char ip[32];
	int port;
	char name[64];
} ntconf_t;

void cfReadGraphic(grconf_t * c);
void cfReadNetwork(ntconf_t *c);

int cfReadGameData(void);
void cfGetTexture(const char *name, texc_t *tex);

shiptype_t * cfGetShip(const char * name);
turrettype_t * cfGetTurret(const char * name);


#endif /* CONFIG_H_ */
