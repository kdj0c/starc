/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdlib.h>
#include <string.h>

#include <math.h>

#include "config.h"
#include "parse.h"
#include "ship.h"

int nbship = 0;
shiptype_t * stype = NULL;
int nbturret = 0;
turrettype_t * ttype = NULL;

#define cfShipString(cst, f, st)  cfReadShipString(cst, #f, st[i].f)
#define cfShipFloat(cst, f, st)   cfReadShipFloat(cst, #f, &st[i].f)

void cfReadGraphic(grconf_t *c) {
    struct ps_node *conf;
    struct ps_node *grconf;


	c->fullscreen = 0;
	c->width = 800;
	c->heigh = 600;

	conf = psParseFile("config.cfg");

	if (!conf) {
		printf("Error when reading configuration file config.cfg\n");
		return;
	}
	grconf = psGetObject("graphic", conf);
	c->fullscreen = psGetInt("fullscreen", grconf);
	c->width = psGetInt("width", grconf);
    c->heigh = psGetInt("heigh", grconf);

	psFreeNodes(conf);
}

void cfReadNetwork(ntconf_t *c) {
    #if 0
	const char *tmp;

	strcpy(c->ip, "127.0.0.1");
	c->port = 1234;
	strcpy(c->name, "player");

	config_init(&conf);
	if (config_read_file(&conf, "config.cfg") == CONFIG_FALSE) {
		printf("config.cfg:%d - %s\n", config_error_line(&conf),
				config_error_text(&conf));
		return;
	}

	config_lookup_string(&conf, "network.ip", &tmp);
	strcpy(c->ip, tmp);
	config_lookup_int(&conf, "network.port", &(c->port));
	config_lookup_string(&conf, "network.name", &tmp);
	strcpy(c->name, tmp);
	config_destroy(&conf);
	#endif
}
#if 0
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

int cfShipGetLaser(config_setting_t *cst, laser_t *las) {
	config_setting_t *csl;
	config_setting_t *csl2;
	int j, nb;

	csl = config_setting_get_member(cst, "laser");
	if (!csl)
		return 0;
	nb = config_setting_length(csl);
	for (j = 0; j < nb; j++) {
		csl2 = config_setting_get_elem(csl, j);
		cfReadShipFloat(csl2, "x", &las[j].p.x);
		cfReadShipFloat(csl2, "y", &las[j].p.y);
		cfReadShipFloat(csl2, "r", &las[j].r);
		las[j].r *= M_PI / 180.;
		config_setting_lookup_int(csl2, "color", (int *) &las[j].color);
	}
	return nb;
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
		cfReadShipFloat(csl2, "x", &st->burst[j].p.x);
		cfReadShipFloat(csl2, "y", &st->burst[j].p.y);
		cfReadShipFloat(csl2, "size", &st->burst[j].size);
		config_setting_lookup_int(csl2, "color", (int *) &st->burst[j].color);
	}
}

void cfShipGetTurret(config_setting_t *cst, shiptype_t *st) {
	config_setting_t *csl;
	config_setting_t *csl2;
	int j;
	const char *tmp;

	csl = config_setting_get_member(cst, "turret");
	if (!csl)
		return;

	st->numturret = config_setting_length(csl);
	for (j = 0; j < st->numturret; j++) {
		csl2 = config_setting_get_elem(csl, j);
		config_setting_lookup_string(csl2, "type", &tmp);
		st->turret[j].t = cfGetTurret(tmp);
		cfReadShipFloat(csl2, "x", &st->turret[j].p.x);
		cfReadShipFloat(csl2, "y", &st->turret[j].p.y);
	}
}

void cfShipGetHangar(config_setting_t *cst, shiptype_t *st) {
	config_setting_t *csl;

	csl = config_setting_get_member(cst, "hangar");
	if (!csl)
		return;

	st->flag |= SH_MOTHERSHIP;
	cfReadShipFloat(csl, "x", &st->hangar.p.x);
	cfReadShipFloat(csl, "y", &st->hangar.p.y);
	cfReadShipFloat(csl, "r", &st->hangar.r);
	st->hangar.r *= M_PI / 180.;
}

int cfReadGameData(void) {
	config_setting_t *cs;
	config_setting_t *cst;
	int i;

	config_init(&conf);
	if (config_read_file(&conf, "ship.cfg") == CONFIG_FALSE) {
		printf("ship.cfg:%d - %s\n", config_error_line(&conf),
				config_error_text(&conf));
		return -1;
	}

	cs = config_lookup(&conf, "turrettypes");
	nbturret = config_setting_length(cs);
	ttype = malloc(sizeof(*ttype) * nbturret);
	memset(ttype, 0, sizeof(*ttype) * nbturret);

	for (i = 0; i < nbturret; i++) {
		cst = config_setting_get_elem(cs, i);
		cfShipString(cst, name, ttype);
		cfShipString(cst, imgfile, ttype);
		cfShipFloat(cst, size, ttype);
		cfShipString(cst, shieldfile, ttype);
		cfShipFloat(cst, shieldsize, ttype);
		cfShipFloat(cst, maxhealth, ttype);
		cfShipFloat(cst, maniability, ttype);
		ttype[i].numlaser = cfShipGetLaser(cst, ttype[i].laser);
	}

	cs = config_lookup(&conf, "shiptypes");
	nbship = config_setting_length(cs);
	stype = malloc(sizeof(*stype) * nbship);
	memset(stype, 0, sizeof(*stype) * nbship);

	for (i = 0; i < nbship; i++) {
		cst = config_setting_get_elem(cs, i);
		cfShipString(cst, name, stype);
		cfShipString(cst, imgfile, stype);
		cfShipFloat(cst, size, stype);
		cfShipString(cst, shieldfile, stype);
		cfShipFloat(cst, shieldsize, stype);
		cfShipFloat(cst, maxhealth, stype);
		cfShipFloat(cst, maniability, stype);
		cfShipFloat(cst, thrust, stype);

		stype[i].numlaser = cfShipGetLaser(cst, stype[i].laser);
		cfShipGetBurst(cst, &stype[i]);
		cfShipGetTurret(cst, &stype[i]);
		cfShipGetHangar(cst, &stype[i]);
	}

	config_destroy(&conf);
	return 0;
}

shiptype_t * cfGetShip(const char * name) {
	int i;
	for (i = 0; i < nbship; i++) {
		if (!strcmp(name, stype[i].name))
			return &stype[i];
	}
	printf("Error cannot find ship type %s\n", name);
	return NULL;

}

turrettype_t * cfGetTurret(const char * name) {
	int i;
	for (i = 0; i < nbturret; i++) {
		if (!strcmp(name, ttype[i].name))
			return &ttype[i];
	}
	printf("Error cannot find turret type %s\n", name);
	return NULL;
}
#endif
