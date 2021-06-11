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

void get_cube_3d(GLvec& p, GLvec & normals, GLfloat lx, GLfloat ly, GLfloat lz)
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

	// compute normal vectors
	// resize: total number of coordinate(n: number of vertecies + 3: number of coordinate part) -> normal vectors
	normals.resize(n * 3);
	// moving cursor pointer
	float* cursor = normals.data();

	// for vertices on the side at z = -0.5
	for (int i = 0; i < 6; ++i, cursor += 3) { cursor[0] = 0; cursor[1] = 0; cursor[2] = -1; }
	// for vertices on the side at x = -0.5
	for (int i = 0; i < 6; ++i, cursor += 3) { cursor[0] = -1; cursor[1] = 0; cursor[2] = 0; }
	// for vertices on the side at y = -0.5
	for (int i = 0; i < 6; ++i, cursor += 3) { cursor[0] = 0; cursor[1] = -1; cursor[2] = 0; }
	// for vertices on the side at z = 0.5
	for (int i = 0; i < 6; ++i, cursor += 3) { cursor[0] = 0; cursor[1] = 0; cursor[2] = 1; }
	// for vertices on the side at x = 0.5
	for (int i = 0; i < 6; ++i, cursor += 3) { cursor[0] = 1; cursor[1] = 0; cursor[2] = 0; }
	// for vertices on the side at y = 0.5
	for (int i = 0; i < 6; ++i, cursor += 3) { cursor[0] = 0; cursor[1] = 1; cursor[2] = 0; }
}

void get_sphere_3d(GLvec& p, GLvec& normals, GLfloat r, GLint subh, GLint suba)
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

				// for normals (vx/r, vy/r, vz/r)
				FPUSH_VTX3(normals, vx0 / r, vy0 / r, vz0 / r);
				FPUSH_VTX3(normals, vx1 / r, vy1 / r, vz1 / r);
				FPUSH_VTX3(normals, vx3 / r, vy3 / r, vz3 / r);
			}

			if (1 < i) {
				// second triangles (v3 - v2 - v0): Counter clockwise
				FPUSH_VTX3(p, vx3, vy3, vz3);
				FPUSH_VTX3(p, vx2, vy2, vz2);
				FPUSH_VTX3(p, vx0, vy0, vz0);

				// for normals (vx/r, vy/r, vz/r)
				FPUSH_VTX3(normals, vx3 / r, vy3 / r, vz3 / r);
				FPUSH_VTX3(normals, vx2 / r, vy2 / r, vz2 / r);
				FPUSH_VTX3(normals, vx0 / r, vy0 / r, vz0 / r);
			}
		}
	}
}

void get_cone_3d(
	GLvec& p,
	GLvec& normals,
	std::vector<size_t>& side_idx,
	std::vector<size_t>& botton_idx,
	GLfloat radius, GLfloat height, GLint n)
{
	GLfloat half_height = height / 2;
	GLfloat theta, x, z;

	FPUSH_VTX3(p, 0, half_height, 0); // top vertex (0, h/2, 0) --> v(0)
	side_idx.push_back(0);  // --> side: v(0)

	// side triangles and bottom triangles --> seperate vertex
	// side triangles: 1 ~ n+1
	for (int i = 0; i <= n; ++i) {
		theta = (GLfloat)(2.0 * M_PI * i / n);
		x = radius * sin(theta);	// x(i): rsin(theta), y(i): -height/2
		z = radius * cos(theta);	// z(i): rcos(theta)
		FPUSH_VTX3(p, x, -half_height, z); // v(i+1) + p(i) -> side: 1 ~ n + 1
		side_idx.push_back(i + 1);
	}

	// bottom triangles: n+2(center)
	FPUSH_VTX3(p, 0, -half_height, 0); // v(n+2)  = (0, -h/2, 0) -> bottom 
	botton_idx.push_back(n + 2);

	// bottom triangles: n+3 ~ 2n+3
	for (int i = n + 2; i <= (2 * n + 2); ++i) {
		theta = (GLfloat)(2.0 * M_PI * i / n);
		x = radius * sin(theta);	// x(i): rsin(theta), y(i): -height/2
		z = radius * cos(theta);	// z(i): rcos(theta)
		FPUSH_VTX3(p, x, -half_height, z); // v(n+3) + p(i) -> bottom: n+2 ~ 2n+3
		botton_idx.push_back(i + 1);
	}


	// ****** normal vector ******
	// v(0) --> normal vector(0, 1, 0)
	FPUSH_VTX3(normals, 0, 1, 0); 

	// side triangles: 1 ~ n+1 --> normal vetor((1/r)(x, 0, z))
	for (int i = 0; i <= n; ++i) {
		theta = (GLfloat)(2.0 * M_PI * i / n);
		x = radius * sin(theta);	
		z = radius * cos(theta);
		FPUSH_VTX3(normals, x / radius, 0, z / radius); // side
	}

	// bottom triangles: n+2 ~ 2n+3 --> normal vector(0, -1, 0)
	for (int i = n + 2; i <= 2 * n + 3; ++i)
		FPUSH_VTX3(normals, 0, -1, 0); 
}

void get_cylinder_3d(
	GLvec& p,
	GLvec& normals,
	std::vector<size_t>& side_idx,
	std::vector<size_t>& top_idx,
	std::vector<size_t>& bottom_idx,
	GLfloat radius, GLfloat height, GLint n)
{
	GLfloat half_height = height / 2;
	GLfloat theta, x, z;
	p.resize(3 * (4 * n + 6));

	// side-top triangles and top triangles --> seperate vertex
	// top triangles(center)
	FPUSH_VTX3_AT(p, 0, 0, half_height, 0); // top(v0) 
	top_idx.push_back(0);
	// top triangles: 1 ~ n+1
	for (int i = 0; i <= n; ++i) {
		theta = (GLfloat)(2.0 * M_PI * i / n);
		x = radius * sin(theta);
		z = radius * cos(theta);
		FPUSH_VTX3_AT(p, i + 1, x, half_height, z); 
		top_idx.push_back(i + 1);
	}
	
	// side triangles: n+2 ~ 3n+3
	// q = n+2 --> side-top: q + 2i
	//		   --> side-bottom: q + (2i+1)
	for (int i = 0; i <= n; ++i) {
		theta = (GLfloat)(2.0 * M_PI * i / n);
		x = radius * sin(theta);
		z = radius * cos(theta);
		FPUSH_VTX3_AT(p, (n + 2) + (i * 2), x, half_height, z); // side-top vertex --> side: triangle strip(rect <- two triangles)
		FPUSH_VTX3_AT(p, (n + 2) + (i * 2 + 1), x, -half_height, z); // side-bottom vertex
		side_idx.push_back((n + 2) + (i * 2));	// side-top
		side_idx.push_back((n + 2) + (i * 2 + 1));	// side-bottom
	}
	
	// side-bottom triangles and bottom triangles --> seperate vertex
	// bottom triangles(center)
	FPUSH_VTX3_AT(p, 3 * n + 4, 0, -half_height, 0); // bottom(v(3n+4)) 
	bottom_idx.push_back(3 * n + 4);

	// bottom triangles: 3n+5 ~ 4n+5
	for (int i = 0; i <= n; ++i) {
		theta = (GLfloat)(2.0 * M_PI * i / n);
		x = radius * sin(theta);
		z = radius * cos(theta);
		FPUSH_VTX3_AT(p, 3 * n + 4 + (i + 1), x, -half_height, z);
		bottom_idx.push_back(3 * n + 4 + (i + 1));
	}
	
	// ****** normal vector ******
	// top triangles: 0 ~ n+1 --> normal vector(0, 1, 0)
	for (int i = 0; i <= n + 1; ++i)
		FPUSH_VTX3(normals, 0, 1, 0);
	
	// side triangles: n+2 ~ 3n+3 --> normal vector((1/r)(x, 1, z))
	for (int i = 0; i <= n; ++i) {
		theta = (GLfloat)(2.0 * M_PI * i / n);
		x = radius * sin(theta);
		z = radius * cos(theta);
		FPUSH_VTX3(normals, x / radius, 1 / radius, z / radius); // side-top
		FPUSH_VTX3(normals, x / radius, 1 / radius, z / radius); // side-bottom
	}

	// bottom triangles: 3n+4 ~ 4n+5 --> normal vector(0, -1, 0)
	for (int i = 3 * n + 4; i <= 4 * n + 5; ++i)
		FPUSH_VTX3(normals, 0, -1, 0);
}

void get_torus_3d(GLvec& p, GLvec& normals,
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
			// normal vetors = (1/r1)(p - (x0, 0, z0)) =  (1/r1)(dx, y, dz)
			FPUSH_VTX3(normals, dx / r1, y / r1, dz / r1);

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

void get_grid(GLvec& p, GLfloat w, GLfloat h, int m, int n)
{
	GLfloat x0 = -0.5f * w;
	GLfloat x1 = +0.5f * w;
	GLfloat z0 = -0.5f * h;
	GLfloat z1 = +0.5f * h;

	for (int i = 0; i <= m; ++i) {
		GLfloat x = x0 + w * i / m;
		FPUSH_VTX3(p, x, 0, z0);
		FPUSH_VTX3(p, x, 0, z1);
	}
	for (int i = 0; i <= n; ++i) {
		GLfloat z = z0 + h * i / n;
		FPUSH_VTX3(p, x0, 0, z);
		FPUSH_VTX3(p, x1, 0, z);
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