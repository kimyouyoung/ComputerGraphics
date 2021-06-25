#pragma once

#include <GL/glew.h>
#include <vector>

typedef std::vector<GLfloat> GLvec;

void get_cube_3d(
	GLvec& p,
	GLfloat lx,
	GLfloat ly,
	GLfloat lz
);

void get_color_3d_by_pos(
	GLvec& c,
	GLvec& p,
	GLfloat offset = 0
);

void get_grid(
	GLvec& p,
	GLfloat w,
	GLfloat h,
	int m, int n, int l
);

void bind_buffer(
	GLuint buffer,
	GLvec& vec,
	int program,
	const GLchar* attri_name,
	GLint attri_size
);