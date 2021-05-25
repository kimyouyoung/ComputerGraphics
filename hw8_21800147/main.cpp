#include <stdio.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "LoadShaders.h"
#define _USE_MATH_DEFINES // required to use M_PI
#include <math.h>
#include <vector>
#include <time.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace std;
using namespace glm;

typedef std::vector<GLfloat> GLvec;
typedef std::vector<size_t> GLsvec;

const GLuint num_of_model = 5;

GLuint vao[num_of_model];
GLuint vbo[num_of_model][2];
GLvec vtx_pos[num_of_model], vtx_clrs[num_of_model];
GLuint element_buffs[2];
GLuint cylinder_element_buffs[3];
vector<GLuint> torus_element_buffs;
GLsvec idx_list[2];
GLsvec side_idx, top_idx, bottom_idx;
GLsvec* cy_idx_list[] = { &side_idx, &top_idx, &bottom_idx };
vector<GLsvec> t_side_idx;

bool show_wireframes = false;
bool show_vertices = false;
bool orth = false;
bool pers = false;
int program;
int projection_mode = 1;

void keyboard(unsigned char key, int x, int y);
void display();
void init(int program);
int build_program();
void bind_buffer(GLuint buffer, GLvec& vec, int program,
	const GLchar* attri_name, GLint attri_size);
mat4 parallel(double r, double aspect, double n, double f);

void get_color_3d_by_pos(GLvec& c, GLvec& p);
void get_cube_3d(GLvec& p, GLfloat lx, GLfloat ly, GLfloat lz);
void draw_cube();
void get_sphere_3d(GLvec& p, GLfloat r, GLint subh, GLint suba);
void draw_sphere();
void get_cone_3d(GLvec& p, GLsvec& side_idx, GLsvec& botton_idx, GLfloat radius, GLfloat height, GLint n);
void draw_cone();
void get_cylinder_3d(GLvec& p, GLsvec& side_idx, GLsvec& top_idx, GLsvec& bottom_idx, GLfloat radius, GLfloat height, GLint n);
void draw_cylinder();
void get_torus_3d(GLvec& p, vector<GLsvec>& side_idx, GLfloat r0, GLfloat r1, GLint na, GLint nh);
void draw_torus();
mat4 transCar(GLfloat sx, GLfloat sy, GLfloat sz, GLfloat tx, GLfloat ty, GLfloat tz, mat4* T_pre = NULL, mat4* T_post = NULL, bool set_uniform = true);
void draw_car();

#define FPUSH_VTX3(p, vx, vy, vz)\
do {\
	p.push_back(vx);\
	p.push_back(vy);\
	p.push_back(vz);\
}while(0)

#define FSET_VTX3(vx, vy, vz, valx, valy, valz)\
do {\
	vx = (float)(valx);\
	vy = (float)(valy);\
	vz = (float)(valz);\
}while(0)

#define FPUSH_VTX3_AT(p, i, vx, vy, vz)\
do {\
	size_t i3 = 3*(i);\
	p[i3+0] = (float)(vx);\
	p[i3+1] = (float)(vy);\
	p[i3+2] = (float)(vz);\
}while(0)

void main(int argc, char** argv)
{
	// initialize GLUT to make a window
	glutInit(&argc, argv);
	// initialize and set the basic window
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(512, 512);
	glutCreateWindow("Drawing Primitives");

	// initialize GLEW to load OpenGL extensions
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	program = build_program();
	init(program);
	// register a display callback function (When rendering is required in openGL)
	glutDisplayFunc(display);

	// keyboard callback function in GLUT
	glutKeyboardFunc(keyboard);
	// enter the GLUT event processing loop.
	glutMainLoop();
}

void keyboard(unsigned char key, int x, int y)
{
	// let the GLUT redraw the screen by calling display. (the correct for keyboard.)
	switch (key) {
		// assign to a according to the number received on the keyboard.
		// 1: cube, 2: sphere, 3: cone, 4: cylinder, 5:tours, 6: composite model
	case '1': projection_mode = 1; glutPostRedisplay(); break;
	case '2': projection_mode = 2; glutPostRedisplay(); break;
	case '3': projection_mode = 3; glutPostRedisplay(); break;
	case '4': projection_mode = 4; glutPostRedisplay(); break;
	case '5': projection_mode = 5; glutPostRedisplay(); break;
	case '6': projection_mode = 6; glutPostRedisplay(); break;

		// w: wireframe, v: vertices, c: colored object
	case 'w': show_wireframes = true; show_vertices = false; glutPostRedisplay(); break;
	case 'v': show_vertices = true; show_wireframes = false; glutPostRedisplay(); break;
	case 'c': show_vertices = false; show_wireframes = false; glutPostRedisplay(); break;

		// o: orthographic, p: perspective
	case 'o': orth = true; pers = false; glutPostRedisplay(); break;
	case 'p': pers = true; orth = false; glutPostRedisplay(); break;

	}
}

void display()
{
	// set the clear color to white
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// draw colored object(fill)
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);

	// 0: fragment shader => fcolor, location: 4 => fragment
	glUniform1i(4, 0);

	mat4 M(1.0f);
	// multiply 0.001f and time to set the angle.
	GLfloat theta = 0.001f * clock();
	M = rotate(M, theta, vec3(-1.0f, 1.0f, 0.0f));
	M = scale(M, vec3(1.0f));

	// transfer values to uniform variables.
	glUniformMatrix4fv(1, 1, GL_FALSE, value_ptr(M));

	// find window's width and height
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);
	double aspect = 1.0 * width / height;

	mat4 V = lookAt(vec3(0, 0, 5), vec3(0, 0, 0), vec3(0, 1, 0));
	// projection (orthographic or perpective)
	mat4 P(1.0);

	if (orth) {
		// for orthographic projection
		P = parallel(1.2, aspect, 0.01, 10.0);
	}
	else {
		// for perspective projection
		P = perspective(M_PI / 180.0 * (30.0), aspect, 0.01, 10.0);
	}

	// viewport values and projection values to uniform variables.
	glUniformMatrix4fv(2, 1, GL_FALSE, value_ptr(V));
	glUniformMatrix4fv(3, 1, GL_FALSE, value_ptr(P));


	if (projection_mode == 1)
		draw_cube();
	else if (projection_mode == 2)
		draw_sphere();
	else if (projection_mode == 3)
		draw_cone();
	else if (projection_mode == 4)
		draw_cylinder();
	else if (projection_mode == 5)
		draw_torus();
	else if (projection_mode == 6)
		draw_car();

	glDisable(GL_POLYGON_OFFSET_FILL);

	// if show_wireframes is true
	if (show_wireframes)
	{
		// draw line on sphere
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(0.7);

		// 1: fragment shader => black line, location: 4 => fragment
		glUniform1i(4, 1);
		if (projection_mode == 1)
			draw_cube();
		else if (projection_mode == 2)
			draw_sphere();
		else if (projection_mode == 3)
			draw_cone();
		else if (projection_mode == 4)
			draw_cylinder();
		else if (projection_mode == 5)
			draw_torus();
		else if (projection_mode == 6)
			draw_car();

	}

	// if show_vertices is true
	if (show_vertices)
	{
		// draw point on sphere
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		glPointSize(2);

		// 1: fragment shader => black point, location: 4 => fragment
		if (projection_mode == 1)
			draw_cube();
		else if (projection_mode == 2)
			draw_sphere();
		else if (projection_mode == 3)
			draw_cone();
		else if (projection_mode == 4)
			draw_cylinder();
		else if (projection_mode == 5)
			draw_torus();
		else if (projection_mode == 6)
			draw_car();
	}

	glFlush();

	// Draw again.
	glutPostRedisplay();
}


void init(int program)
{
	srand(clock());

	// make 3d cube
	get_cube_3d(vtx_pos[0], 1, 1, 1);
	// make 3d sphere(radius: 0.8f, subdivition height: 20, subdivision axis: 25)
	get_sphere_3d(vtx_pos[1], 0.8f, 20, 25);
	// make 3d cone(radius: 0.6f, subdivition height: 1.2, subdivisions: 10)
	get_cone_3d(vtx_pos[2], idx_list[0], idx_list[1], 0.6f, 1.2, 10);
	// make 3d cylinder(radius: 0.6f, subdivition height: 1.6, subdivisions: 10)
	get_cylinder_3d(vtx_pos[3], side_idx, top_idx, bottom_idx, 0.6f, 1.6, 10);
	// make 3d torus(tours radius: 0.3f, section radius: 0.4, subdivisions axis: 10, subdivisions height: 10)
	get_torus_3d(vtx_pos[4], t_side_idx,  0.3, 0.4, 10, 10);

	torus_element_buffs.resize(t_side_idx.size());

	for (int i = 0; i < 5; i++) {
		// make objects colors using vertex positons
		get_color_3d_by_pos(vtx_clrs[i], vtx_pos[i]);
		// generate a new vertex array object
		glGenVertexArrays(1, &vao[i]);
		// initialize vertex buffers to be referenced by vao.
		glBindVertexArray(vao[i]);


		const GLchar* attri_name[2] = { "vPosition", "vColor" };
		GLvec* vtx_list[2] = { &(vtx_pos[i]), &(vtx_clrs[i]) };

		// generate a new buffer objects.
		glGenBuffers(2, vbo[i]);
		for (int j = 0; j < 2; ++j) {
			// bind buffer objects each for vertex and color
			bind_buffer(vbo[i][j], *vtx_list[j], program, attri_name[j], 3);
		}

		// generate a new buffer objects for 3D cone.
		glGenBuffers(2, element_buffs);
		for (int j = 0; j < 2; ++j) {
			// bind buffer for element buffs -> cone
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffs[j]);
			// copy idx_list data to the buffer object
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(size_t) * idx_list[j].size(), idx_list[j].data(), GL_STATIC_DRAW);
		}

		// generate a new buffer objects for 3D cylinder.
		glGenBuffers(2, cylinder_element_buffs);
		for (int j = 0; j < 3; ++j) {
			// bind buffer for cylinder_element_buffs -> cylinder
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylinder_element_buffs[j]);
			// copy cy_idx_list data to the buffer object
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(size_t) * (cy_idx_list[j]->size()), cy_idx_list[j]->data(), GL_STATIC_DRAW);
		}

		// generate a new buffer objects for 3D torus.
		// get size from torus element buffer size.
		size_t n = torus_element_buffs.size();
		glGenBuffers(n, torus_element_buffs.data());
		for (int j = 0; j < n; ++j) {
			// bind buffer for torus_element_buffs -> torus
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, torus_element_buffs[j]);
			// copy t_side_idx data to the buffer object
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(size_t) * (t_side_idx[j].size()), t_side_idx[j].data(), GL_STATIC_DRAW);
		}

	}

	// active depth test
	glEnable(GL_DEPTH_TEST);
	// pass if the depth value of the frame is less than the stored depth value.
	glDepthFunc(GL_LESS);
}

int build_program()
{
	// compile shaders and produce a program to which the complied shaders are attached. Then, register the program in OpenGL.
	ShaderInfo shaders[] = {
		{GL_VERTEX_SHADER, "vao_example.vert"},
		{GL_FRAGMENT_SHADER, "vao_example.frag"},
		{GL_NONE, NULL}
	};
	GLuint program = LoadShaders(shaders);
	// shader activation
	glUseProgram(program);
	return program;
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

mat4 parallel(double r, double aspect, double n, double f)
{
	double l = -r;
	double width = 2 * r;
	double height = width / aspect;
	double t = height / 2;
	double b = -t;
	// for orthographic projection
	return ortho(l, r, b, t, n, f);
}

void get_color_3d_by_pos(GLvec& c, GLvec& p)
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

void get_cube_3d(GLvec& p, GLfloat lx, GLfloat ly, GLfloat lz)
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

void draw_cube()
{
	glBindVertexArray(vao[0]);
	glDrawArrays(GL_TRIANGLES, 0, vtx_pos[0].size() / 3);
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

void draw_sphere()
{
	glBindVertexArray(vao[1]);
	// draw triangles
	glDrawArrays(GL_TRIANGLES, 0, vtx_pos[1].size() / 3);
}

void get_cone_3d(GLvec& p, GLsvec& side_idx, GLsvec& botton_idx, GLfloat radius, GLfloat height, GLint n)
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

void draw_cone()
{
	glBindVertexArray(vao[2]);
	for (int i = 0; i < 2; ++i) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffs[i]);
		glDrawElements(GL_TRIANGLE_FAN, idx_list[i].size(), GL_UNSIGNED_INT, NULL);
	}
}

void get_cylinder_3d(GLvec& p, GLsvec& side_idx, GLsvec& top_idx, GLsvec& bottom_idx, GLfloat radius, GLfloat height, GLint n)
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

void draw_cylinder()
{
	// strip for side part, triangle_strip for top and bottom
	GLuint drawing_mode[3] = { GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLE_FAN };

	glBindVertexArray(vao[3]);
	for (int i = 0; i < 3; ++i) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylinder_element_buffs[i]);
		glDrawElements(drawing_mode[i], cy_idx_list[i]->size(), GL_UNSIGNED_INT, NULL);
	}
}

void get_torus_3d(GLvec& p, vector<GLsvec>& side_idx, GLfloat r0, GLfloat r1, GLint na, GLint nh)
{

	side_idx.resize(nh);
	GLfloat gamma, y, l;
	GLfloat theta;
	GLfloat x, z, dx, dz;

	for (int i = nh, j = 0; i >= 0; --i, ++j) {
		GLfloat gamma = (GLfloat)(2.0 * M_PI * j / nh);
		y = r1 * sin(gamma);
		l = r1 * cos(gamma);
		int a = j * (na + 1);
		int b = (j + 1) * (na + 1);
		for (int k = 0; k <= na; ++k) {
			theta = (GLfloat)(2.0 * M_PI * k / na);
			x = (r0 + r1) * sin(theta);
			z = (r0 + r1) * cos(theta);
			dx = l * sin(theta);
			dz = l * cos(theta);

			FPUSH_VTX3(p, x + dx, y, z + dz); // only side

			if (j < nh) {
				t_side_idx[j].push_back(a + k);		// index list for even th
				t_side_idx[j].push_back(b + k);		// index list for odd th
			}
		}
	}
}

void draw_torus()
{
	glBindVertexArray(vao[4]);
	size_t n = torus_element_buffs.size();
	for (int i = 0; i < n; ++i) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, torus_element_buffs[i]);
		glDrawElements(GL_TRIANGLE_STRIP, t_side_idx[i].size(), GL_UNSIGNED_INT, NULL);
	}
}

mat4 transCar(GLfloat sx, GLfloat sy, GLfloat sz, GLfloat tx, GLfloat ty, GLfloat tz, mat4* T_pre, mat4* T_post, bool set_uniform)
{
	mat4 T;
	T = translate(T, vec3(tx, ty, tz));
	T = scale(T, vec3(sx, sy, sz));
	if (T_pre) T = (*T_pre) * T;
	if (T_post) T = T * (*T_post);
	if (set_uniform) {
		glUniformMatrix4fv(1, 1, GL_FALSE, value_ptr(T));
	}

	return T;
}

void draw_car()
{
	GLfloat theta = 0.001f * clock();
	mat4 Rz = rotate(mat4(), -2 * theta, vec3(0.0f, 0.0f, 1.0f));
	mat4 Ry = rotate(mat4(), -theta, vec3(0.0f, 1.0f, 0.0f));

	// car body (cube)
	transCar(1.2f, 0.4f, 0.6f, +0.0f, -0.2f, 0.0f, &Ry);
	draw_cube();
	// car upper body
	transCar(0.6f, 0.6f, 0.6f, -0.3f, +0.3f, 0.0f, &Ry);
	draw_cube();
	// car front body
	mat4 R_fb = rotate(mat4(), radians(90.0f), vec3(0, 0, 1)) * Ry;
	transCar(0.5f, 0.5f, 0.5f, +0.25f, 0.0f, 0.0f, &Ry, &R_fb);
	draw_cylinder();

	// car roof
	transCar(1.0f, 0.2f, 1.0f, -0.3f, +0.7f, 0.0f, &Ry);
	draw_cone();

	// car front-right light
	transCar(0.1f, 0.1f, 0.1f, +0.6f, -0.2f, -0.2f, &Ry);
	draw_sphere();
	// car front-left light
	transCar(0.1f, 0.1f, 0.1f, +0.6f, -0.2f, +0.2f, &Ry);
	draw_sphere();

	// front left tire
	mat4 R_tire = Rz * rotate(mat4(), radians(90.0f), vec3(1, 0, 0));
	transCar(0.3f, 0.3f, 0.3f, +0.3f, -0.4f, -0.4f, &Ry, &R_tire);
	draw_torus();
	// front right tire
	transCar(0.3f, 0.3f, 0.3f, +0.3f, -0.4f, +0.4f, &Ry, &R_tire);
	draw_torus();
	// rear left tire
	transCar(0.3f, 0.3f, 0.3f, -0.3f, -0.4f, -0.4f, &Ry, &R_tire);
	draw_torus();
	// rear right tire
	transCar(0.3f, 0.3f, 0.3f, -0.3f, -0.4f, +0.4f, &Ry, &R_tire);
	draw_torus();

	// front shaft
	mat4 R_shaft = Rz * rotate(mat4(), radians(90.0f), vec3(1, 0, 0));
	transCar(0.12f, 0.12f, 0.9f, +0.3f, -0.4f, +0.0f, &Ry, &R_shaft);
	draw_cylinder();
	// rear shaft
	transCar(0.12f, 0.12f, 0.9f, -0.3f, -0.4f, +0.0f, &Ry, &R_shaft);
	draw_cylinder();

}

