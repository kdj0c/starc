/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>

#include "graphic.h"
#include "ship.h"

/* For testing only */
#define NBPART 10000

typedef struct {
	float x;
	float y;
	float dx;
	float dy;
	int texture;
	float c;
	int maxlife;
	int life;
} particle_t;

particle_t parts[NBPART];

void add_explosion(float x, float y, float v) {
	int i;
	GLuint texture;

	texture = grLoadTexture("img/particle.png");

	for (i = 0; i < NBPART; i++) {
		float rx,ry;
		rx = ((float) (((rand() % 1000) - 500) * (rand() % 1000 - 500) / (500.0 * 500.0)));
		parts[i].maxlife = rand() % 1000 + 5000;
		ry = ((float) (((rand() % 1000) - 500) * (rand() % 1000 - 500) / (500.0 * 500.0)));
		parts[i].x = x + rx * 10.0;
		parts[i].y = y + ry * 10.0;
		parts[i].dx = rx * v;
		parts[i].dy = ry * v;
		parts[i].life = parts[i].maxlife;
		parts[i].texture = texture;
	}
}

void update_particle(void) {
	int i;
	grSetBlendAdd(parts[0].texture);
	for (i = 0; i < NBPART; i++) {
		parts[i].x += parts[i].dx;
		parts[i].y += parts[i].dy;
		parts[i].life -= 1;
		parts[i].c = (float) parts[i].life / (float) parts[i].maxlife;
		if (parts[i].life > 0 )
			grBlitSquare( parts[i].x,parts[i].y,10.0,parts[i].c);
	}
}


void grDraw(void) {
	static int timebase = 0;
	static int frame = 0;
	int time;
	float fps;

	glutTimerFunc(10,grDraw,0);
	frame++;
	time = glutGet(GLUT_ELAPSED_TIME);

	if (time - timebase > 1000) {
		fps = frame*1000.0/((float)(time-timebase));
		printf("fps %f\n",fps);
		timebase = time;
		frame = 0;
	}

	glClear(GL_COLOR_BUFFER_BIT );
    glColor4f(1.0, 1.0, 1.0, 1.0);
	update_particle();
	shUpdateShips(10);
	shDrawShips();
	glutSwapBuffers();
}

int main(int argc, char *argv[], char *envp[]) {
	int WindowName;

    srand (1983);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	WindowName = glutCreateWindow("test particle");
	glutFullScreen();

	glutReshapeFunc(grReshape);
	glutDisplayFunc(grDraw);
	glutTimerFunc(10,grDraw,0);
	add_explosion(5000.0,3000.0,1.0);
	shLoadShip();
	shCreateShip("v2", 5000,3000,0);

	glutMainLoop();
	return 0;
}
