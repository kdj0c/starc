#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef _WIN32
#define GL_GLEXT_PROTOTYPES
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include "shader.h"

static char *read_file(const char *file) {
	int fd;
	size_t size = 4096;
	size_t len;
	char *buf;

	fd = open(file, O_RDONLY);
	assert(fd >= 0);

	buf = malloc(size);
	assert(buf);
	len = read(fd, buf, size);
	assert(len > 0);
	assert(len < size);
	buf[len] = 0;
	return buf;
}

static void print_shader_info_log(GLuint shader_index) {
	int max_length = 2048;
	int actual_length = 0;
	char log[2048];
	glGetShaderInfoLog(shader_index, max_length, &actual_length, log);
	printf("shader info log for GL index %i:\n%s\n", shader_index, log);
}

static GLuint create_shader(const char *file_name, GLenum type) {
	printf("creating shader from %s...\n", file_name);
	const GLchar *shader_str;
	GLuint shader;

	shader_str = read_file(file_name);

	shader = glCreateShader(type);
	shader_str = (const GLchar *) read_file(file_name);
	glShaderSource(shader, 1, &shader_str, NULL);
	glCompileShader(shader);
	// check for compile errors
	int params = -1;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		printf("ERROR: GL shader index %i did not compile\n", shader);
		print_shader_info_log(shader);
		return 0;
	}
	printf("shader compiled. index %i\n", shader);
	return shader;
}

void print_programme_info_log(GLuint sp) {
	int max_length = 2048;
	int actual_length = 0;
	char log[2048];
	glGetProgramInfoLog(sp, max_length, &actual_length, log);
	printf("program info log for GL index %u:\n%s", sp, log);
}

static GLint is_programme_valid(GLuint sp) {
	glValidateProgram(sp);
	GLint params = -1;
	glGetProgramiv(sp, GL_VALIDATE_STATUS, &params);
	if (GL_TRUE != params) {
		printf("program %i GL_VALIDATE_STATUS = GL_FALSE\n", sp);
		print_programme_info_log(sp);
		return params;
	}
	printf("program %i GL_VALIDATE_STATUS = GL_TRUE\n", sp);
	return params;
}

GLuint create_programme(GLuint vert, GLuint frag) {
	GLuint programme;
	programme = glCreateProgram();
	printf("created programme %u. attaching shaders %u and %u...\n", programme, vert, frag);
	glAttachShader(programme, vert);
	glAttachShader(programme, frag);
	// link the shader programme. if binding input attributes do that before link
	glLinkProgram(programme);

	assert(is_programme_valid(programme) == GL_TRUE);

	// delete shaders here to free memory
	glDeleteShader(vert);
	glDeleteShader(frag);
	return programme;
}

GLuint create_programme_from_files(const char *vert_file_name, const char *frag_file_name) {
	GLuint vert, frag;
	vert = create_shader(vert_file_name, GL_VERTEX_SHADER);
	frag = create_shader(frag_file_name, GL_FRAGMENT_SHADER);
	return create_programme(vert, frag);
}

 /*******************/
