#pragma once

#include <GL/glew.h>
#include <vector>

typedef std::vector<GLfloat> GLvec;

void get_cube_3d(
	GLvec& p,
	GLvec& normals,
	GLfloat lx,
	GLfloat ly,
	GLfloat lz
);

void get_sphere_3d(
	GLvec& p,
	GLvec& normals,
	GLfloat r,
	GLint lats,
	GLint longs
);

void get_cone_3d(
	GLvec& p,
	GLvec& normals,
	std::vector<size_t>& side_idx,
	std::vector<size_t>& bottom_idx,
	GLfloat radius,
	GLfloat height,
	GLint subdivs
);

void get_cylinder_3d(
	GLvec& p,
	GLvec& normals,
	std::vector<size_t>& side_idx,
	std::vector<size_t>& top_idx,
	std::vector<size_t>& bottom_idx,
	GLfloat radius,
	GLfloat height,
	GLint subdivs
);

void get_torus_3d(
	GLvec& p,
	GLvec& normals,
	std::vector<std::vector<size_t>>& side_idx,
	GLfloat radius0,
	GLfloat radius1,
	GLint longs,
	GLint lats
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
	int m, int n
);

void bind_buffer(
	GLuint buffer,
	GLvec& vec,
	int program,
	const GLchar* attri_name,
	GLint attri_size
);