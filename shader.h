#define GL_GLEXT_PROTOTYPES

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

GLuint create_programme_from_files(	const char* vert_file_name, const char* frag_file_name);
