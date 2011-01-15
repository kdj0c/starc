/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>

#include "menu.h"
#include "gamemain.h"
#include "graphic.h"
#include "config.h"

int main(int argc, char *argv[], char *envp[]) {
	int WindowName;
	grconf_t c;

	srand(1983);

	cfReadGraphic(&c);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	WindowName = glutCreateWindow("StarC");

	meInitMenu();

	glutReshapeFunc(grReshape);
	glutDisplayFunc(meDisplayMenu);

	glutReshapeWindow(c.width,c.heigh);

	if (c.fullscreen)
		glutFullScreen();

	meInitMenu();

/*	aiCreate(shCreateShip("v1", 10000, 0, -1, 1));
	aiCreate(shCreateShip("v1", 10000, 900, -1, 1));
	aiCreate(shCreateShip("v2", 0, 900, 0, 0));
	aiCreate(shCreateShip("v2", 0, -900, 0, 0));
	aiCreate(shCreateShip("v1", -10000, -900, 0, 1));
	aiCreate(shCreateShip("v1", -10000, -1800, 0, 1));
	aiCreate(shCreateShip("v2", 0, 50000, 0, 0));*/
	glutMainLoop();
	return 0;
}
