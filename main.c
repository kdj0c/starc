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

#include "graphic.h"
#include "ship.h"
#include "ai.h"
#include "star.h"

ship_t * player;
ai_t * ai;

/* For testing only */
#define NBPART 100

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
		float len,angle,rx,ry;

		len = (float) ((rand() % 1000) - 500) / 500.f;
		angle = (float) (rand() % 1000) * M_PI / 500.f;
		rx = len * cos(angle);
		ry = len * sin(angle);
		parts[i].maxlife = rand() % 1000;
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
			grBlitSquare( parts[i].x,parts[i].y,100.0,parts[i].c);
	}
}

void dummy() {

}

void grDraw(int value) {
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

	grChangeview(player->x,player->y,player->r);

	glClear(GL_COLOR_BUFFER_BIT ); //Efface le frame buffer et le Z-buffer
    glColor4f(1.0, 1.0, 1.0, 1.0);
    stUpdate(player->x,player->y);
    stBlit();
	update_particle();
	aiThink(ai);
	shUpdateShips(10);
	shDrawShips();
	glutSwapBuffers();
}

void keyup(unsigned char key, int x, int y) {
	switch(key) {
	case 'c':
		player->in.acceleration = 0;
		break;
	case 'h':
	case 'n':
		player->in.direction = 0;
		break;
	case ' ':
		player->in.fire1 = 0;
	}
}
void keydown(unsigned char key, int x, int y){
	switch(key) {
	case 27:
		exit(0);
	case 'c':
		player->in.acceleration = 1;
		break;
	case 'h':
		player->in.direction = 1;
		break;
	case 'n':
		player->in.direction = -1;
		break;
	case ' ':
		player->in.fire1 = 1;
	}
}

int main(int argc, char *argv[], char *envp[]) {
	int WindowName;

    srand (1983);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	WindowName = glutCreateWindow("test particle");
	glutFullScreen();

	glutReshapeFunc(grReshape);
	glutDisplayFunc(dummy);
	glutKeyboardUpFunc(keyup);
	glutKeyboardFunc(keydown);
	add_explosion(0,0,5.0);
	shLoadShip();
	//player = shCreateShip("v2", 5000,3000,0);
	player = shCreateShip("v2", 0,0,0,0);
	ai = aiCreate(shCreateShip("v1",900,900,0,1),player);
	shCreateShip("v2", 0,900,0,0);
	shCreateShip("v2", 900,0,0,0);
	shCreateShip("v2", -900,0,0,0);
	glutTimerFunc(10,grDraw,0);
	glutMainLoop();
	return 0;
}
