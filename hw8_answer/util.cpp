#include "util.h"
#define _USE_MATH_DEFINES
#include <math.h>

#define FPUSH_VTX3(p, vx, vy, vz)\
			do {\
				p.push_back(vx);\
				p.push_back(vy);\
				p.push_back(vz);\
			}while(0)

#define FPUSH_VTX3_AT(p, i, vx, vy, vz)\
			do {\
				size_t i3 = 3*(i);\
				p[i3+0] = (float)(vx);\
				p[i3+1] = (float)(vy);\
				p[i3+2] = (float)(vz);\
			}while(0)

#define FSET_VTX3(vx, vy, vz, valx, valy, valz)\
			do {\
				vx = (float)(valx);\
				vy = (float)(valy);\
				vz = (float)(valz);\
			}while(0)

void get_box_3d(GLvec & p, GLfloat lx, GLfloat ly, GLfloat lz)
{
	static const GLfloat box_vertices[] = {
		0.5f,0.5f,-0.5f, -0.5f,-0.5f,-0.5f, -0.5f,0.5f,-0.5f,	// side at z = -0.5
		0.5f,0.5f,-0.5f, 0.5f,-0.5f,-0.5f, -0.5f,-0.5f,-0.5f,

		-0.5f,-0.5f,-0.5f, -0.5f,-0.5f,0.5f, -0.5f,0.5f,0.5f,	// side at x = -0.5
		-0.5f,-0.5f,-0.5f, -0.5f,0.5f,0.5f, -0.5f,0.5f,-0.5f,

		0.5f,-0.5f,0.5f, -0.5f,-0.5f,-0.5f, 0.5f,-0.5f,-0.5f,	// side at y = -0.5
		0.5f,-0.5f,0.5f, -0.5f,-0.5f,0.5f, -0.5f,-0.5f,-0.5f,

		-0.5f,0.5f,0.5f, -0.5f,-0.5f,0.5f, 0.5f,-0.5f,0.5f,		// side at z = 0.5
		0.5f,0.5f,0.5f, -0.5f,0.5f,0.5f, 0.5f,-0.5f,0.5f,

		0.5f,0.5f,0.5f, 0.5f,-0.5f,-0.5f, 0.5f,0.5f,-0.5f,		// side at x = 0.5
		0.5f,-0.5f,-0.5f, 0.5f,0.5f,0.5f, 0.5f,-0.5f,0.5f,

		0.5f,0.5f,0.5f, 0.5f,0.5f,-0.5f, -0.5f,0.5f,-0.5f,		// side at y = 0.5
		0.5f,0.5f,0.5f, -0.5f,0.5f,-0.5f, -0.5f,0.5f,0.5f
	};

	// adjust the vector "p" to the size of the "box_vertices".
	p.resize(sizeof(box_vertices) / sizeof(GLfloat));
	// copy the contents of "box_vertices" into vector "p".
	memcpy(p.data(), box_vertices, sizeof(box_vertices));
	// multiply the index for the x-axis[0], y-axis[1], and z-axis[2] by lx, ly, and lz, respectively.
	size_t n = p.size() / 3;
	for (int i = 0; i < n; ++i)
	{
		p[3 * i + 0] *= lx;
		p[3 * i + 1] *= ly;
		p[3 * i + 2] *= lz;
	}
}

void get_sphere_3d(GLvec& p, GLfloat r, GLint subh, GLint suba)
{
	for (int i = 1; i <= subh; ++i) {
		double theta0 = M_PI * (i - 1) / subh;
		double theta1 = M_PI * i / subh;

		// rst = rsin(theta)
		// vx = sin(phi)rst
		// vy = rcos(theta)
		// vz = cos(phi)rst
		// calculate: rst and rcos(theta)
		double y0 = r * cos(theta0);
		double rst0 = r * sin(theta0);
		double y1 = r * cos(theta1);
		double rst1 = r * sin(theta1);

		for (int j = 1; j <= suba; ++j) {
			double phi0 = 2 * M_PI * (j - 1) / suba;
			double phi1 = 2 * M_PI * j / suba;

			// vx = sin(phi)rst
			// vy = rcos(theta)
			// vz = cos(phi)rst
			// calculate: sin(phi) and cos(phi)
			double cp0 = cos(phi0);
			double sp0 = sin(phi0);
			double cp1 = cos(phi1);
			double sp1 = sin(phi1);

			float vx0, vy0, vz0, vx1, vy1, vz1;
			float vx2, vy2, vz2, vx3, vy3, vz3;

			// compute the coordinates(v0, v1, v2, v3)
			FSET_VTX3(vx0, vy0, vz0, sp0 * rst0, y0, cp0 * rst0);
			FSET_VTX3(vx1, vy1, vz1, sp0 * rst1, y1, cp0 * rst1);
			FSET_VTX3(vx2, vy2, vz2, sp1 * rst0, y0, cp1 * rst0);
			FSET_VTX3(vx3, vy3, vz3, sp1 * rst1, y1, cp1 * rst1);

			if (i < subh) {
				// first triangles (v0 - v1 - v3): Counter clockwise
				FPUSH_VTX3(p, vx0, vy0, vz0);
				FPUSH_VTX3(p, vx1, vy1, vz1);
				FPUSH_VTX3(p, vx3, vy3, vz3);
			}

			if (1 < i) {
				// second triangles (v3 - v2 - v0): Counter clockwise
				FPUSH_VTX3(p, vx3, vy3, vz3);
				FPUSH_VTX3(p, vx2, vy2, vz2);
				FPUSH_VTX3(p, vx0, vy0, vz0);
			}
		}
	}
}

void get_cone_3d(
	GLvec& p, 
	std::vector<size_t>& side_idx, 
	std::vector<size_t>& botton_idx, 
	GLfloat radius, GLfloat height, GLint n)
{
	GLfloat half_height = height / 2;
	GLfloat theta, x, z;

	FPUSH_VTX3(p, 0, half_height, 0); // top vertex (0, h/2, 0)
	side_idx.push_back(0);
	for (int i = 0; i <= n; ++i) {
		theta = (GLfloat)(2.0 * M_PI * i / n);
		x = radius * sin(theta);	// x(i): rsin(theta), y(i): -height/2, z(i): rcos(theta)
		z = radius * cos(theta);
		FPUSH_VTX3(p, x, -half_height, z); // v(i+1) + p(i) -> side
		side_idx.push_back(i + 1);
		botton_idx.push_back(n + 2 - i);
	}
	FPUSH_VTX3(p, 0, -half_height, 0); // v(i+2)  = (0, -h/2, 0) -> bottom 
	botton_idx.push_back(1);
}

void get_cylinder_3d(
	GLvec& p, 
	std::vector<size_t>& side_idx, 
	std::vector<size_t>& top_idx, 
	std::vector<size_t>& bottom_idx, 
	GLfloat radius, GLfloat height, GLint n)
{
	GLfloat half_height = height / 2;
	GLfloat theta, x, z;
	p.resize(3 * (2 * n + 4));

	FPUSH_VTX3_AT(p, 0, 0, half_height, 0); // top 
	top_idx.push_back(0);
	bottom_idx.push_back(2 * n + 3);
	for (int i = 0; i <= n; ++i) {
		theta = (GLfloat)(2.0 * M_PI * i / n);
		x = radius * sin(theta);
		z = radius * cos(theta);
		FPUSH_VTX3_AT(p, 2 * i + 1, x, half_height, z); // side-top vertex --> side: triangle strip(rect <- two triangles)
		FPUSH_VTX3_AT(p, 2 * i + 2, x, -half_height, z); // side-bottom vertex
		side_idx.push_back(2 * i + 1);	// side-top
		side_idx.push_back(2 * i + 2);	// side=bottom
		top_idx.push_back(2 * i + 1);
		bottom_idx.push_back(2 * n + 2 - 2 * i);
	}
	FPUSH_VTX3_AT(p, 2 * n + 3, 0, -half_height, 0); // bottom 
}

void get_torus_3d(GLvec& p, 
	std::vector<std::vector<size_t>>& side_idx, 
	GLfloat r0, GLfloat r1, 
	GLint longs, GLint lats)
{

	side_idx.resize(lats);

	for (int j = lats, k = 0; j >= 0; --j, ++k) {
		GLfloat phi = (GLfloat)(2.0 * M_PI * j / lats);
		GLfloat y = r1 * sin(phi);
		GLfloat l = r1 * cos(phi);
		int a = k * (longs + 1);
		int b = (k + 1) * (longs + 1);
		for (int i = 0; i <= longs; ++i) {
			GLfloat theta = (GLfloat)(2.0 * M_PI * i / longs);
			GLfloat st = sin(theta);
			GLfloat ct = cos(theta);
			GLfloat x0 = (r0 + r1) * st;
			GLfloat z0 = (r0 + r1) * ct;
			GLfloat dx = l * st;
			GLfloat dz = l * ct;

			FPUSH_VTX3(p, x0 + dx, y, z0 + dz);

			if (k < lats) {
				side_idx[k].push_back(a + i);
				side_idx[k].push_back(b + i);
			}
		}
	}
}

void get_color_3d_by_pos(GLvec& c, GLvec& p, GLfloat offset)
{
	GLfloat max_val[3] = { -INFINITY, -INFINITY, -INFINITY };
	GLfloat min_val[3] = { INFINITY, INFINITY, INFINITY };

	// compute the bounding box
	int n = (int)(p.size() / 3);
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < 3; ++j) {
			GLfloat val = p[i * 3 + j];
			if (max_val[j] < val) max_val[j] = val;
			else if (min_val[j] > val) min_val[j] = val;
		}
	}

	GLfloat width[3] = {
		// x width
		max_val[0] - min_val[0],
		// y width
		max_val[1] - min_val[1],
		// z width
		max_val[2] - min_val[2]
	};

	// normalize the coordinates
	c.resize(p.size());
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < 3; ++j) {
			int k = i * 3 + j;
			c[k] = fminf((p[k] - min_val[j]) / width[j], 1.0f);
		}
	}
}

void bind_buffer(GLuint buffer, GLvec& vec, int program,
	const GLchar* attri_name, GLint attri_size)
{
	// bind the buffer object to a specific type.
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	// copy vertex data to the buffer object.
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vec.size(),
		vec.data(), GL_STATIC_DRAW);
	// returns the index of a general vertex attribute bound to that attribute variable.
	GLint location = glGetAttribLocation(program, attri_name);
	// the specified vertex shader attribute variable and the data information stored in the vertex buffer object are stored in the VAO.
	glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0);
	// active location
	glEnableVertexAttribArray(location);
}