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

#define GL_GLEXT_PROTOTYPES

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>

#include "shader.h"
#include "gamemain.h"


/*
#include "menu.h"
#include "gamemain.h"
#include "graphic.h"
#include "config.h"
*/

int g_gl_width = 640;
int g_gl_height = 480;
SDL_Window* g_window;

SDL_Joystick *joy = NULL;

void gamepad(void) {
	int numjoy;

	numjoy = SDL_NumJoysticks();
	printf("sdl found %d joy\n", numjoy);
	//for (j = 0; j < numjoy; j++) {
	if (numjoy > 0) {
		joy = SDL_JoystickOpen(0);
		printf("joystick #%d, %s opened\n", 0, SDL_JoystickName(0));
	}
	//}
}

void exitCleanup(void) {
	printf("cleanup before exiting\n");
	if (joy)
		SDL_JoystickClose(joy);
	SDL_Quit();
	exit(0);
}

void start_gl (void) {
	SDL_GLContext glContext;
    const GLubyte* renderer;
    const GLubyte* version;

	/* Glew will later ensure that OpenGL 3 *is* supported on this machine */


  /* Initialise SDL - when using C/C++ it's common to have to
	 initialise libraries by calling a function within them. */
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK | SDL_INIT_TIMER)<0) {
		  fprintf(stderr, "Failed to initialise SDL: %s\n", SDL_GetError());
		  exit(1);
	}


	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG, GL_TRUE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	/* Do double buffering in GL */
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	/* When we close a window quit the SDL application */
	atexit(SDL_Quit);

	/* Create a new window with an OpenGL surface */
	g_window = SDL_CreateWindow("starc"
							  , SDL_WINDOWPOS_CENTERED
							  , SDL_WINDOWPOS_CENTERED
							  , g_gl_width
							  , g_gl_height
							  , SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (!g_window) {
		fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
		exit(1);
	}

	glContext = SDL_GL_CreateContext(g_window);
	if (!glContext) {
		fprintf(stderr, "Failed to create OpenGL context: %s\n", SDL_GetError());
		exit(1);
	}

    /* get version info */
    renderer = glGetString (GL_RENDERER); /* get renderer string */
    version = glGetString (GL_VERSION); /* version as a string */
    printf ("Renderer: %s\n", renderer);
    printf ("OpenGL version supported %s\n", version);
}

void sdl_error_callback (int error, const char* description) {
	fputs (description, stderr);
	//gl_log_err ("%s\n", description);
}

/* a call-back function */
void sdl_window_size_callback (SDL_Window* window, int width, int height) {
	g_gl_width = width;
	g_gl_height = height;
	SDL_SetWindowSize(window, width, height);
	printf ("width %i height %i\n", width, height);
	/* update any perspective matrices used here */
}

unsigned int LoadTexture(char * filename) {
	unsigned int textureHandle;
	SDL_Surface *sdlsurf;

	sdlsurf = IMG_Load(filename);

	glGenTextures(1, &textureHandle);
	glActiveTexture (GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sdlsurf->w, sdlsurf->h, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, sdlsurf->pixels);

	return textureHandle;
}

	GLuint vbo;
	GLuint vao;
void init_graph(void) {
	GLfloat points[] = {
		 0.0f,	0.0f,	0.0f,
		 0.5f,  0.0f,	0.0f,
		 0.5f,  0.5f,	0.0f,
		 0.0f,  0.5f,   0.0f
	};

	GLfloat texcoords[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};
	/* tell GL to only draw onto a pixel if the shape is closer to the viewer */
//	glEnable (GL_DEPTH_TEST); /* enable depth-testing */
//	glDepthFunc (GL_LESS); /* depth-testing interprets a smaller value as "closer" */


	glGenBuffers (1, &vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glBufferData (GL_ARRAY_BUFFER, 12 * sizeof (GLfloat), points, GL_STATIC_DRAW);

	GLuint texcoords_vbo;
	glGenBuffers (1, &texcoords_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, texcoords_vbo);
	glBufferData (GL_ARRAY_BUFFER, 8 * sizeof (GLfloat), texcoords, GL_STATIC_DRAW);


	glGenVertexArrays (1, &vao);
	glBindVertexArray (vao);

	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer (GL_ARRAY_BUFFER, texcoords_vbo);
	glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray (0);
	glEnableVertexAttribArray (1);
}

unsigned int tick(unsigned int interval, void *param) {
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.code = 0;
	event.user.data1 = 0;
	event.user.data2 = 0;
	SDL_PushEvent(&event);
	return interval;
}



int main(int argc, char *argv[], char *envp[]) {
	GLuint shader_programme;
	SDL_Event event;
	GLint colour_loc;
//	grconf_t c;
//	SDL_VideoInfo *info;

	start_gl();

	gmStartSingle();

    exit(0);

	init_graph();

	shader_programme = create_programme_from_files ("shaders/basic_tex_vs.glsl", "shaders/basic_tex_fs.glsl");

//	colour_loc = glGetUniformLocation (shader_programme, "inputColour");
	glUseProgram (shader_programme);
//	glUniform4f (colour_loc, 1.0f, 1.0f, 0.0f, 1.0f);

	LoadTexture("img/v1.png");

//	glUniform4f (colour_loc, 1.0f, 0.0f, 0.0f, 1.0f);

	/* Call the function "tick" every 1/60th second */
	SDL_AddTimer(1000/60, tick, NULL);

	/* The main event loop */
	while (SDL_WaitEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			SDL_Quit();
			break;
		case SDL_USEREVENT:
			/* wipe the drawing surface clear */
			glClear (GL_COLOR_BUFFER_BIT);
			glViewport (0, 0, g_gl_width, g_gl_height);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glUseProgram (shader_programme);
			glBindVertexArray (vao);
			/* draw points 0-3 from the currently bound VAO with current in-use shader */
			glDrawArrays (GL_TRIANGLE_FAN, 0, 4);

			/* put the stuff we've been drawing onto the display */
			SDL_GL_SwapWindow(g_window);
			break;
		case SDL_WINDOWEVENT:
			switch (event.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					sdl_window_size_callback(g_window, event.window.data1, event.window.data2);
					break;
				default:
					break;
			}
		default:
			break;
		}
	}



	/*	info = SDL_GetVideoInfo();
	 *printf("fullscreen resolution %d %d\n",info->current_w,
	 *info->current_h); */

	return 0;
}
