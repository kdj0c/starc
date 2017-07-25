/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <math.h>

#include "config.h"
#include "parse.h"
#include "ship.h"

int nbship = 0;
shiptype_t * stype = NULL;
int nbturret = 0;
turrettype_t * ttype = NULL;

#define cfShipString(cf, f, st)  strcpy(st[i].f, psGetStr(#f, cf))

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


int cfShipGetLaser(struct ps_node *cfg, laser_t *las) {
    struct ps_node *lcfg;
	int j;

	lcfg = psGetObject("laser", cfg);
	j = 0;

	for (lcfg = lcfg->child; lcfg; lcfg = lcfg->next) {
        las[j].p.x = psGetFloat("x", lcfg);
        las[j].p.y = psGetFloat("y", lcfg);
        las[j].r = psGetFloat("r", lcfg) * M_PI / 180.;
        las[j].color = (unsigned int) psGetInt("color", lcfg);
        j++;
	}
	return j;
}

void cfShipGetBurst(struct ps_node *cfg, shiptype_t *st) {
    struct ps_node *lcfg;
	int j;

	lcfg = psGetObject("burst", cfg);
	j = 0;

	for (lcfg = lcfg->child; lcfg; lcfg = lcfg->next) {
        st->burst[j].p.x = psGetFloat("x", lcfg);
        st->burst[j].p.y = psGetFloat("y", lcfg);
        st->burst[j].size = psGetFloat("size", lcfg);
        st->burst[j].color = (unsigned int) psGetInt("color", lcfg);
        j++;
	}
	st->numburst = j;
}

void cfShipGetTurret(struct ps_node *cfg, shiptype_t *st) {
    struct ps_node *tcfg;
	int j;

	tcfg = psGetObject("turret", cfg);
	if (!tcfg)
        return;

	j = 0;
	for (tcfg = tcfg->child; tcfg; tcfg = tcfg->next) {
        st->turret[j].t = cfGetTurret(psGetStr("type", tcfg));
        st->turret[j].p.x = psGetFloat("x", tcfg);
        st->turret[j].p.y = psGetFloat("y", tcfg);
        j++;
	}
	st->numturret = j;
}

void cfShipGetHangar(struct ps_node *cfg, shiptype_t *st) {
    struct ps_node *hcfg;

    hcfg = psGetObject("hangar", cfg);
    if (!hcfg)
        return;

	st->flag |= SH_MOTHERSHIP;
	st->hangar.p.x = psGetFloat("x", hcfg);
	st->hangar.p.y = psGetFloat("y", hcfg);
	st->hangar.r = psGetFloat("r", hcfg) * M_PI / 180.;
}

int cfReadGameData(void) {
    struct ps_node *conf;
    struct ps_node *tcfg;
    struct ps_node *scfg;
    int i;

    conf = psParseFile("ship.cfg");

	if (!conf) {
		printf("Error when reading configuration file ship.cfg\n");
		return -1;
	}

	tcfg = psGetObject("turrettypes", conf);
	nbturret = tcfg->len;
	ttype = malloc(sizeof(*ttype) * nbturret);
	memset(ttype, 0, sizeof(*ttype) * nbturret);
    tcfg = tcfg->child;

    for (i = 0; i < nbturret && tcfg; i++) {
        cfShipString(tcfg, name, ttype);
        cfShipString(tcfg, imgfile, ttype);
        ttype[i].size = psGetFloat("size", tcfg);
        cfShipString(tcfg, shieldfile, ttype);
        ttype[i].shieldsize = psGetFloat("shieldsize", tcfg);
        ttype[i].maxhealth = psGetFloat("maxhealth", tcfg);
        ttype[i].maniability = psGetFloat("maniability", tcfg);
        ttype[i].numlaser = cfShipGetLaser(tcfg, ttype[i].laser);
        tcfg = tcfg->next;
    }

    scfg = psGetObject("shiptypes", conf);
	nbship = scfg->len;
	stype = malloc(sizeof(*stype) * nbship);
	memset(stype, 0, sizeof(*stype) * nbship);
    scfg = scfg->child;

	for (i = 0; i < nbship; i++) {
        cfShipString(scfg, name, stype);
        cfShipString(scfg, imgfile, stype);
        stype[i].size = psGetFloat("size", scfg);
        cfShipString(scfg, shieldfile, stype);
        stype[i].shieldsize = psGetFloat("shieldsize", scfg);
        stype[i].maxhealth = psGetFloat("maxhealth", scfg);
        stype[i].maniability = psGetFloat("maniability", scfg);
        stype[i].thrust = psGetFloat("thrust", scfg);

        stype[i].numlaser = cfShipGetLaser(scfg, stype[i].laser);

		cfShipGetBurst(scfg, &stype[i]);
		cfShipGetTurret(scfg, &stype[i]);
		cfShipGetHangar(scfg, &stype[i]);
		scfg = scfg->next;
	}

    psFreeNodes(conf);
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

