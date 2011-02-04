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
#include <math.h>


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
		printf("config.cfg:%d - %s\n",
				config_error_line(&conf), config_error_text(&conf));
		return;
	}

	config_lookup_int(&conf,"graphic.fullscreen", &c->fullscreen);
	config_lookup_int(&conf,"graphic.width", &c->width);
	config_lookup_int(&conf,"graphic.heigh", &c->heigh);

	config_destroy(&conf);
}

void cfReadNetwork(ntconf_t *c) {
	const char *tmp;
	c->ip = "127.0.0.1";
	c->port = 1234;
	c->name = "player";

	config_init(&conf);
	if (config_read_file(&conf, "config.cfg") == CONFIG_FALSE) {
		printf("config.cfg:%d - %s\n", config_error_line(&conf),
				config_error_text(&conf));
		return;
	}

	config_lookup_string(&conf, "network.ip", &tmp);
	strcpy(c->ip, tmp);
	config_lookup_int(&conf, "network.port", (long *) &(c->port));
	config_lookup_string(&conf, "network.name", &tmp);
	strcpy(c->name, tmp);
	config_destroy(&conf);
}

void cfReadShipString(config_setting_t *cst, const char * name, char * dest) {
	const char *tmp;
	config_setting_lookup_string(cst, name, &tmp);
	strcpy(dest, tmp);
}

void cfReadShipFloat(config_setting_t *cst, const char * name, float * dest) {
	double tmp = 0;
	config_setting_lookup_float(cst, name, &tmp);
	*dest = (float) tmp;
}

void cfShipGetLaser(config_setting_t *cst, shiptype_t *st) {
	config_setting_t *csl;
	config_setting_t *csl2;
	int j;

	csl = config_setting_get_member(cst, "laser");
	if (!csl)
		return;
	st->numlaser = config_setting_length(csl);
	for (j = 0; j < st->numlaser; j++) {
		csl2 = config_setting_get_elem(csl, j);
		cfReadShipFloat(csl2, "x", &st->laser[j].x);
		cfReadShipFloat(csl2, "y", &st->laser[j].y);
		cfReadShipFloat(csl2, "r", &st->laser[j].r);
		st->laser[j].r *= M_PI / 180.;
		config_setting_lookup_int(csl2, "color", (long *) &st->laser[j].color);
	}
}

void cfShipGetBurst(config_setting_t *cst, shiptype_t *st) {
	config_setting_t *csl;
	config_setting_t *csl2;
	int j;

	csl = config_setting_get_member(cst, "burst");
	if (!csl)
		return;

	st->numburst = config_setting_length(csl);
	for (j = 0; j < st->numburst; j++) {
		csl2 = config_setting_get_elem(csl, j);
		cfReadShipFloat(csl2, "x", &st->burst[j].x);
		cfReadShipFloat(csl2, "y", &st->burst[j].y);
		cfReadShipFloat(csl2, "size", &st->burst[j].size);
		config_setting_lookup_int(csl2, "color", (long *) &st->burst[j].color);
	}
}

void cfTurretGetLaser(config_setting_t *cst, turrettype_t *tu) {
	config_setting_t *csl;
	config_setting_t *csl2;
	int j;

	csl = config_setting_get_member(cst, "laser");
	if (!csl)
		return;
	tu->numlaser = config_setting_length(csl);
	for (j = 0; j < tu->numlaser; j++) {
		csl2 = config_setting_get_elem(csl, j);
		cfReadShipFloat(csl2, "x", &tu->laser[j].x);
		cfReadShipFloat(csl2, "y", &tu->laser[j].y);
		cfReadShipFloat(csl2, "r", &tu->laser[j].r);
		tu->laser[j].r *= M_PI / 180.;
		config_setting_lookup_int(csl2, "color", (long *) &tu->laser[j].color);
	}
}

void cfShipGetTurret(config_setting_t *cst, shiptype_t *st) {
	config_setting_t *csl;
	config_setting_t *csl2;
	int j;

	csl = config_setting_get_member(cst, "turret");
	if (!csl)
		return;

	st->numturret = config_setting_length(csl);
	for (j = 0; j < st->numturret; j++) {
		csl2 = config_setting_get_elem(csl, j);
		cfReadShipFloat(csl2, "x", &st->turret[j].x);
		cfReadShipFloat(csl2, "y", &st->turret[j].y);
		cfReadShipFloat(csl2, "maxhealth", &st->turret[j].maxhealth);
		cfReadShipFloat(csl2, "maniability", &st->turret[j].maniability);
		cfTurretGetLaser(csl2, &st->turret[j]);
	}
}

void cfShipGetHangar(config_setting_t *cst, shiptype_t *st) {
	config_setting_t *csl;

	csl = config_setting_get_member(cst, "hangar");
	if (!csl)
		return;

	st->flag |= SH_MOTHERSHIP;
	cfReadShipFloat(csl, "x", &st->hangar.x);
	cfReadShipFloat(csl, "y", &st->hangar.y);
	cfReadShipFloat(csl, "r", &st->hangar.r);
	st->hangar.r *= M_PI / 180.;
}

shiptype_t * cfReadShip(int *num) {
	config_setting_t *cs;
	config_setting_t *cst;
	shiptype_t *st;
	int i, nbship;

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

		cfShipGetLaser(cst, &st[i]);
		cfShipGetBurst(cst, &st[i]);
		cfShipGetTurret(cst, &st[i]);
		cfShipGetHangar(cst, &st[i]);
	}
	config_destroy(&conf);
	return st;
}

