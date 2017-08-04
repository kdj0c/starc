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
#include "weapon.h"

int nbweapon = 0;
weapontype_t *wtype = NULL;
int nbship = 0;
shiptype_t *stype = NULL;
int nbturret = 0;
turrettype_t *ttype = NULL;

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
		printf("config.cfg:%d - %s\n", config_error_line(&conf), config_error_text(&conf));
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

typedef struct {
	char name[64];
	int x;
	int y;
	int w;
	int h;
} atlas_t;

static atlas_t *atlas;
static int nbAtlas;

void cfReadAtlasData(void) {
	struct ps_node *conf;
	struct ps_node *scfg;
	int i;

	conf = psParseFile("img/atlas.cfg");

	if (!conf) {
		printf("Error when reading configuration file atlas.cfg\n");
		return;
	}
	scfg = psGetObject("images", conf);

	nbAtlas = scfg->len;
	printf("Atlas config :%d sprites\n", nbAtlas);
	atlas = malloc(sizeof(*atlas) * nbAtlas);
	memset(atlas, 0, sizeof(*atlas) * nbAtlas);
	scfg = scfg->child;
	for (i = 0; i < nbAtlas && scfg; i++) {
		cfShipString(scfg, name, atlas);
		atlas[i].x = psGetInt("x", scfg);
		atlas[i].y = psGetInt("y", scfg);
		atlas[i].w = psGetInt("width", scfg);
		atlas[i].h = psGetInt("height", scfg);
		scfg = scfg->next;
	}
}

atlas_t *getTexture(const char *name) {
	int i;

	for (i = 0; i < nbAtlas; i++) {
		if (!strcmp(atlas[i].name, name))
			return &atlas[i];
	}
	printf("Error texture sprite not found %s\n", name);
	return NULL;
}

void cfGetTexture(const char *name, texc_t *tex) {
	atlas_t *a;
	int i;
	float texc[8];

	a = getTexture(name);
	tex->index = 0;
	tex->w = (float) a->w;
	tex->h = (float) a->h;

	//printf("%s %d %d %d %d\n", name, a->x, a->y, a->w, a->h);
	texc[0] = a->x;
	texc[1] = a->y;
	texc[2] = a->x + a->w;
	texc[3] = a->y;
	texc[4] = a->x + a->w;
	texc[5] = a->y + a->h;
	texc[6] = a->x;
	texc[7] = a->y + a->h;

	for (i = 0; i < 8; i++)
		tex->texc[i] = texc[i] / 4096.;
}

weapontype_t *cfGetWeapon(const char *name) {
	int i;

	for (i = 0; i < nbweapon; i++) {
		if (!strcmp(wtype[i].name, name))
			return &wtype[i];
	}
	printf("Error weapon type not found %s\n", name);
	return NULL;
}


int cfShipGetWeapons(struct ps_node *cfg, weapon_t *las) {
	struct ps_node *lcfg;
	int j;

	lcfg = psGetObject("weapon", cfg);
	j = 0;

	for (lcfg = lcfg->child; lcfg; lcfg = lcfg->next) {
		las[j].p.x = psGetFloat("x", lcfg);
		las[j].p.y = psGetFloat("y", lcfg);
		las[j].r = psGetFloat("r", lcfg) * M_PI / 180.;
		las[j].wt = cfGetWeapon(psGetStr("type", lcfg));
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

int cfGetWeaponType(struct ps_node *cfg) {
	const char *name;

	name = psGetStr("type", cfg);

	if(!strcmp(name, "laser"))
		return WE_LASER;

	if(!strcmp(name, "missile"))
		return WE_MISSILE;

	return WE_LASER;
}

int cfReadGameData(void) {
	struct ps_node *conf;
	struct ps_node *wcfg;
	struct ps_node *scfg;
	int i;

	conf = psParseFile("ship.cfg");

	if (!conf) {
		printf("Error when reading configuration file ship.cfg\n");
		return -1;
	}

	wcfg = psGetObject("weapontypes", conf);
	nbweapon = wcfg->len;
	wtype = malloc(sizeof(*wtype) * nbweapon);
	memset(wtype, 0, sizeof(*wtype) * nbweapon);
	wcfg = wcfg->child;

	for (i = 0; i < nbweapon && wcfg; i++) {
		cfShipString(wcfg, name, wtype);
		cfGetTexture(wtype[i].name, &wtype[i].texture);
		wtype[i].damage = psGetInt("damage", wcfg);
		wtype[i].firerate = 1000. / psGetFloat("firerate", wcfg);
		wtype[i].speed = psGetFloat("speed", wcfg);
		wtype[i].lifetime = psGetFloat("lifetime", wcfg) * 1000.; // convert seconds to milliseconds.
		wtype[i].color = psGetInt("color", wcfg);
		wtype[i].type = cfGetWeaponType(wcfg);
		wcfg = wcfg->next;
	}

	scfg = psGetObject("shiptypes", conf);
	nbship = scfg->len;
	stype = malloc(sizeof(*stype) * nbship);
	memset(stype, 0, sizeof(*stype) * nbship);
	scfg = scfg->child;

	for (i = 0; i < nbship; i++) {
		cfShipString(scfg, name, stype);
		cfGetTexture(psGetStr("imgfile", scfg), &stype[i].texture);
		// double the size of the sprite
		stype[i].size = 2 * sqrt(stype[i].texture.w * stype[i].texture.w + stype[i].texture.h * stype[i].texture.h);
		cfGetTexture(psGetStr("shieldfile", scfg), &stype[i].shieldtexture);
		stype[i].shieldsize = stype[i].size * 1.3f;
		stype[i].maxhealth = psGetFloat("maxhealth", scfg);
		stype[i].maniability = psGetFloat("maniability", scfg) / 10000.;
		stype[i].thrust = psGetFloat("thrust", scfg) / 10000.;

		stype[i].numweapon = cfShipGetWeapons(scfg, stype[i].laser);

		cfShipGetBurst(scfg, &stype[i]);
		cfShipGetTurret(scfg, &stype[i]);
		cfShipGetHangar(scfg, &stype[i]);
		scfg = scfg->next;
	}

	psFreeNodes(conf);
	return 0;
}

shiptype_t *cfGetShip(const char *name) {
	int i;
	for (i = 0; i < nbship; i++) {
		if (!strcmp(name, stype[i].name))
			return &stype[i];
	}
	printf("Error cannot find ship type %s\n", name);
	return NULL;

}

turrettype_t *cfGetTurret(const char *name) {
	int i;
	for (i = 0; i < nbturret; i++) {
		if (!strcmp(name, ttype[i].name))
			return &ttype[i];
	}
	printf("Error cannot find turret type %s\n", name);
	return NULL;
}
