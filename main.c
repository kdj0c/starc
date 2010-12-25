/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>

#include "pnglite.h"

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

GLuint LoadTexture(char * filename) {
	png_t tex;
	unsigned char* data;
	GLuint textureHandle;

	png_init(0, 0);
	png_open_file_read(&tex, filename);

	data = (unsigned char*) malloc(tex.width * tex.height * tex.bpp);
	png_get_data(&tex, data);

	glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glEnable(GL_TEXTURE_2D);

	png_close_file(&tex);
	free(data);
	return textureHandle;
}


void add_explosion(float x, float y, float v) {
	int i;
	GLuint texture;

	texture = LoadTexture("img/particle.png");

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
void set_texBlendadd(GLuint text) {
	glBindTexture(GL_TEXTURE_2D, text);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void blit_add(GLuint text, float x,float y, float size, float c) {
    glColor4f(c,c,0,c);
    glBegin(GL_QUADS);
    glTexCoord2f(0., 0.);
    glVertex2f(x - size, y);
    glTexCoord2f(0., 1.);
    glVertex2f(x , y + size);
    glTexCoord2f(1., 1.);
    glVertex2f(x + size, y);
    glTexCoord2f(1., 0.);
    glVertex2f(x , y - size);
    glEnd();
}

void update_particle(void) {
	int i;
	set_texBlendadd(parts[0].texture);
	for (i = 0; i < NBPART; i++) {
		parts[i].x += parts[i].dx;
		parts[i].y += parts[i].dy;
		parts[i].life -= 1;
		parts[i].c = (float) parts[i].life / (float) parts[i].maxlife;
		if (parts[i].life > 0 )
			blit_add(parts[i].texture, parts[i].x,parts[i].y,10.0,parts[i].c);
	}
}

void Reshape(int width, int height) {
	glViewport(0, 0, width, height);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glOrtho (0, width, height, 0, 0, 1);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
    printf("w,%d, h, %d\n",width,height);
}

void Draw() {
	static int timebase = 0;
	static int frame = 0;
	int time;
	float fps;

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

	glutSwapBuffers();

	glutPostRedisplay();
}

int main(int argc, char *argv[], char *envp[]) {
	int WindowName;

    srand (1983);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	WindowName = glutCreateWindow("test particle");
	glutFullScreen();

	glutReshapeFunc(Reshape);
	glutDisplayFunc(Draw);

	add_explosion(1000.0,800.0,1.0);
	glutMainLoop();

	return 0;

}
