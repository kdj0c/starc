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

typedef enum {
	pc_up,
	pc_left,
	pc_down,
	pc_right,
} partcon_e;


int nbweapon = 0;
weapontype_t *wtype = NULL;
int nbship = 0;
shiptype_t *stype = NULL;
int nbturret = 0;
turrettype_t *ttype = NULL;
int nbparts = 0;
parttype_t *ptype = NULL;

#define cfShipString(cf, f, st)  strcpy(st[i].f, psGetStr(#f, cf))

static inline unsigned int rgb2bgr(unsigned int c) {
	return 0xFF000000 | ((c << 16) & 0xFF0000) | (c & 0xFF00) | (c >> 16);
}

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

	conf = psParseFile("img/atlas.json");

	if (!conf) {
		printf("Error when reading configuration file img/atlas.json\n");
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
		atlas[i].w = psGetInt("w", scfg);
		atlas[i].h = psGetInt("h", scfg);
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

void cfGetStationParts(struct ps_node *conf) {
	struct ps_node *pcfg;
	int i;

	pcfg = psGetObject("stationparts", conf);
	nbparts = pcfg->len;
	ptype = malloc(sizeof(*ptype) * nbparts);
	memset(ptype, 0, sizeof(*ptype) * nbparts);
	pcfg = pcfg->child;

	for (i = 0; i < nbparts && pcfg; i++) {
		cfShipString(pcfg, name, ptype);
		cfGetTexture(ptype[i].name, &ptype[i].tex);
		ptype[i].maxhealth = psGetFloat("maxhealth", pcfg);
		ptype[i].cmask = psGetInt("cmask", pcfg);
		pcfg = pcfg->next;
	}
}

parttype_t *cfGetPart(const char *name) {
	int i;

	for (i = 0; i < nbparts; i++) {
		if (!strcmp(ptype[i].name, name))
			return &ptype[i];
	}
	printf("Error station part not found %s\n", name);
	return NULL;
}

vec_t getOffset(partcon_e dir, texc_t *tex) {

	switch (dir) {
	case pc_up:
		return vec(tex->h / 2. - 1.0, 0.);
	case pc_left:
		return vec(0., tex->w / 2. - 1.0);
	case pc_down:
		return vec(-tex->h / 2. - 1.0, 0.);
	case pc_right:
		return vec(0., -tex->w / 2. - 1.0);
	default:
		printf("Part connection error %d\n", dir);
		return vec(0.,0.);
	}
}

float getAngle(partcon_e dir1, partcon_e dir2) {

return dir1 * M_PI_2 - dir2 * M_PI_2 + M_PI;
}


int cfAssembleParts(struct ps_node *cfg, part_t * part) {
	struct ps_node *pcfg;
	int npart = 0;
	int p = 0;

	pcfg = psGetObject("parts", cfg);
	if (!pcfg)
		return 0;

	for (pcfg = pcfg->child; pcfg; pcfg = pcfg->next) {
		part[npart].part = cfGetPart(pcfg->value_s);
		npart++;
	}
	pcfg = psGetObject("links", cfg);
	if (!pcfg)
		return 0;

	part[0].p.x = 0.;
	part[0].p.y = 0.;
	part[0].r = 0.;

	for (pcfg = pcfg->child; pcfg; pcfg = pcfg->next) {
		int p1, a1, a2;

		p++;

		p1 = psGetInt("p", pcfg) - 1;
		a1 = psGetInt("a1", pcfg) - 1;
		a2 = psGetInt("a2", pcfg) - 1;

		if (p1 < 0 || p1 >= npart || p >= npart)
			printf("error link is not valid \n");

		if (!(part[p1].part->cmask & (1 << a1)))
			printf("error in link %d, anchor %d not usable mask %08x, %s\n", p1, a1, part[p1].part->cmask, part[p1].part->name);

		if (!(part[p].part->cmask & (1 << a2)))
			printf("error in link %d, anchor %d not usable mask %08x, %s\n", p, a2, part[p].part->cmask, part[p].part->name);

		part[p].p = vmatrix(part[p1].p, getOffset(a1, &part[p1].part->tex), -part[p1].r);
		part[p].r = part[p1].r + getAngle(a1, a2);
		part[p].p = vmatrix(part[p].p, getOffset(a2, &part[p].part->tex), M_PI - part[p].r);
	}
	return npart;
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
	if (!lcfg)
		return 0;

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

int cfShipGetBurst(struct ps_node *cfg, shiptype_t *st) {
	struct ps_node *lcfg;
	int j;

	lcfg = psGetObject("burst", cfg);
	if (!lcfg)
		return 0;

	j = 0;
	for (lcfg = lcfg->child; lcfg; lcfg = lcfg->next) {
		st->burst[j].p.x = psGetFloat("x", lcfg);
		st->burst[j].p.y = psGetFloat("y", lcfg);
		st->burst[j].size = psGetFloat("size", lcfg);
		st->burst[j].color = rgb2bgr(psGetInt("color", lcfg));
		j++;
	}
	return j;
}

int cfShipGetTurret(struct ps_node *cfg, shiptype_t *st) {
	struct ps_node *tcfg;
	int j;

	tcfg = psGetObject("turret", cfg);
	if (!tcfg)
		return 0;

	j = 0;
	for (tcfg = tcfg->child; tcfg; tcfg = tcfg->next) {
		st->turret[j].t = cfGetTurret(psGetStr("type", tcfg));
		st->turret[j].p.x = psGetFloat("x", tcfg);
		st->turret[j].p.y = psGetFloat("y", tcfg);
		j++;
	}
	return j;
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

	if (!strcmp(name, "laser"))
		return WE_LASER;

	if (!strcmp(name, "missile"))
		return WE_MISSILE;

	return WE_LASER;
}

int cfReadGameData(void) {
	struct ps_node *conf;
	struct ps_node *wcfg;
	struct ps_node *tcfg;
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
		wtype[i].lifetime = psGetFloat("lifetime", wcfg) * 1000.;	// convert seconds to milliseconds.
		wtype[i].color = rgb2bgr(psGetInt("color", wcfg));
		wtype[i].type = cfGetWeaponType(wcfg);
		wcfg = wcfg->next;
	}

	tcfg = psGetObject("turrettypes", conf);
	nbturret = tcfg->len;
	ttype = malloc(sizeof(*ttype) * nbturret);
	memset(ttype, 0, sizeof(*ttype) * nbturret);
	tcfg = tcfg->child;

	for (i = 0; i < nbturret && tcfg; i++) {
		cfShipString(tcfg, name, ttype);
		cfShipString(tcfg, imgfile, ttype);
		cfGetTexture(ttype[i].imgfile, &ttype[i].tex);
		ttype[i].size = sqrt(ttype[i].tex.w * ttype[i].tex.w + ttype[i].tex.h * ttype[i].tex.h);
		cfGetTexture("shield", &ttype[i].shieldtex);
		ttype[i].shieldcolor = rgb2bgr(psGetInt("shieldcolor", tcfg));
		ttype[i].shieldsize = ttype[i].size * 1.3f;
		ttype[i].maxhealth = psGetFloat("maxhealth", tcfg);
		ttype[i].maniability = psGetFloat("maniability", tcfg) / 10000.;
		ttype[i].numweapon = cfShipGetWeapons(tcfg, ttype[i].laser);
		tcfg = tcfg->next;
	}

	cfGetStationParts(conf);

	scfg = psGetObject("shiptypes", conf);
	nbship = scfg->len;
	stype = malloc(sizeof(*stype) * nbship);
	memset(stype, 0, sizeof(*stype) * nbship);
	scfg = scfg->child;

	for (i = 0; i < nbship; i++) {
		cfShipString(scfg, name, stype);
		cfGetTexture(psGetStr("imgfile", scfg), &stype[i].texture);
		// double the size of the sprite
		stype[i].size = sqrt(stype[i].texture.w * stype[i].texture.w + stype[i].texture.h * stype[i].texture.h);
		cfGetTexture("shield", &stype[i].shieldtexture);
		stype[i].shieldcolor = rgb2bgr(psGetInt("shieldcolor", scfg));
		stype[i].shieldsize = stype[i].size * 1.3f;
		stype[i].maxhealth = psGetFloat("maxhealth", scfg);
		stype[i].maniability = psGetFloat("maniability", scfg) / 10000.;
		stype[i].thrust = psGetFloat("thrust", scfg) / 10000.;

		stype[i].numweapon = cfShipGetWeapons(scfg, stype[i].laser);
		stype[i].numburst = cfShipGetBurst(scfg, &stype[i]);
		stype[i].numturret = cfShipGetTurret(scfg, &stype[i]);
		cfShipGetHangar(scfg, &stype[i]);
		stype[i].numparts = cfAssembleParts(scfg, stype[i].part);

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
