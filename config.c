/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdlib.h>
#include <string.h>
#include <libconfig.h>


#include "config.h"
#include "ship.h"

config_t conf;

#define cfShipString(cst, f, st)  cfReadShipString(cst, #f, st[i].f)
#define cfShipFloat(cst, f, st)   cfReadShipFloat(cst, #f, &st[i].f)

void cfReadGraphic(grconf_t *c) {
	c->fullscreen = 0;
	c->width = 800;
	c->heigh = 600;

	config_init(&conf);
	if (config_read_file(&conf,"config.cfg") == CONFIG_FALSE) {
		printf("ship.cfg:%d - %s\n",
				config_error_line(&conf), config_error_text(&conf));
		return;
	}

	config_lookup_int(&conf,"graphic.fullscreen", &c->fullscreen);
	config_lookup_int(&conf,"graphic.width", &c->width);
	config_lookup_int(&conf,"graphic.heigh", &c->heigh);

	config_destroy(&conf);
}

void cfReadShipString(config_setting_t *cst, const char * name, char * dest) {
	const char *tmp;
	config_setting_lookup_string(cst, name, &tmp);
	strcpy(dest, tmp);
}

void cfReadShipFloat(config_setting_t *cst, const char * name, float * dest) {
	double tmp;
	config_setting_lookup_float(cst, name, &tmp);
	*dest = (float) tmp;
}

shiptype_t * cfReadShip(int *num) {
	config_setting_t *cs;
	config_setting_t *cst;
	config_setting_t *csl;
	config_setting_t *csl2;
	shiptype_t *st;
	int i, nbship, j;

	*num = 0;

	config_init(&conf);
	if (config_read_file(&conf, "ship.cfg") == CONFIG_FALSE) {
		printf("ship.cfg:%d - %s\n",
				config_error_line(&conf), config_error_text(&conf));
		return NULL;
	}

	cs = config_lookup(&conf, "shiptypes");
	nbship = config_setting_length(cs);
	st = malloc(sizeof(*st) * nbship);
	memset(st, 0, sizeof(*st) * nbship);
	*num = nbship;

	for (i = 0; i < nbship; i++) {
		cst = config_setting_get_elem(cs, i);
		cfShipString(cst, name, st);
		cfShipString(cst, imgfile, st);
		cfShipFloat(cst, size, st);
		cfShipString(cst, shieldfile, st);
		cfShipFloat(cst, shieldsize, st);
		cfShipFloat(cst, maxhealth, st);
		cfShipFloat(cst, maniability, st);
		cfShipFloat(cst, thrust, st);

		csl = config_setting_get_member(cst, "laser");
		st[i].numlaser = config_setting_length(csl);
		for (j = 0; j < st[i].numlaser; j++) {
			csl2 = config_setting_get_elem(csl, j);
			cfReadShipFloat(csl2, "x", &st[i].laser[j].x);
			cfReadShipFloat(csl2, "y", &st[i].laser[j].y);
			cfReadShipFloat(csl2, "r", &st[i].laser[j].r);
			config_setting_lookup_int(csl2, "color",(long *) &st[i].laser[j].color);
		}
		csl = config_setting_get_member(cst, "burst");
		st[i].numburst = config_setting_length(csl);
		for (j = 0; j < st[i].numburst; j++) {
			csl2 = config_setting_get_elem(csl, j);
			cfReadShipFloat(csl2, "x", &st[i].burst[j].x);
			cfReadShipFloat(csl2, "y", &st[i].burst[j].y);
			config_setting_lookup_int(csl2, "color",(long *) &st[i].burst[j].color);
		}
	}
	config_destroy(&conf);
	return st;
}

