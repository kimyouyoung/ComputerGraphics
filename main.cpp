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

const GLuint num_of_model = 6;

GLuint vao[num_of_model];
GLuint vbo[num_of_model][2];
GLvec vtx_pos[num_of_model], vtx_clrs[num_of_model];
GLuint element_buffs[2];
GLsvec idx_list[2];

bool show_wireframes = false;
bool show_vertices = false;
int program;
int projection_mode = 0;

void keyboard(unsigned char key, int x, int y);
void display();
void init(GLuint program, GLfloat radius, GLfloat height, GLuint subdivs);
int build_program();
void bind_buffer(GLuint buffer, GLvec& vec, int program,
	const GLchar* attri_name, GLint attri_size);
mat4 parallel(double r, double aspect, double n, double f);

void get_color_3d_by_pos(GLvec& c, GLvec& p);
void get_cube_3d(GLvec& p, GLfloat lx, GLfloat ly, GLfloat lz);
void draw_cube(const GLfloat* trans_mat);
void get_sphere_3d(GLvec& p, GLfloat r, GLint subh, GLint suba);
void draw_sphere(const GLfloat* trans_mat);
void get_cone_3d(GLvec& p, GLsvec& side_idx, GLsvec& botton_idx, GLfloat radius, GLfloat height, GLint n);
void draw_cone(GLuint vao);
void get_cylinder_3d(GLvec& p, GLsvec& side_idx, GLsvec& top_idx, GLsvec& bottom_idx, GLfloat radius, GLfloat height, GLint n);
void draw_cylinder(const GLfloat* trans_mat);
void get_tours_3d(GLvec& p, vector<GLsvec>& side_idx, GLfloat r0, GLfloat r1, GLint na, GLint nh);
void draw_tours(const GLfloat* trans_mat);

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
	init(program, 30, 50, 10);
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
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// multiply 0.001f and time to set the angle.
	GLfloat theta = 0.001f * clock();
	
	GLuint location = glGetUniformLocation(program, "draw_mode");
	// set the value of uniform.
	glUniform1i(location, projection_mode);

	// bind vertex buffers to be referenced by vao.(each sun, earth, and moon vao)
	glBindVertexArray(vao[projection_mode]);
	mat4 T(1.0f);
	if (projection_mode == 1) {
		// rotate cube
		T = rotate(T, theta, vec3(1.0f, 0.0f, 0.0f));
		T = rotate(T, theta, vec3(0.0f, 0.1f, 0.0f));
		T = rotate(T, theta, vec3(1.0f, 0.0f, 0.0f));
	}
	else if (projection_mode == 2) {
		// rotate sphere
		T = rotate(T, theta, vec3(1.0f, 0.0f, 0.0f));
		T = rotate(T, theta, vec3(0.0f, 0.1f, 0.0f));
		T = rotate(T, theta, vec3(1.0f, 0.0f, 0.0f));
	}
	else if (projection_mode == 3) {
		T = rotate(T, theta, vec3(1.0f, 0.0f, 0.0f));
	}


	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);
	double aspect = 1.0 * width / height;

	mat4 V = lookAt(vec3(0, 0, 5), vec3(0, 0, 0), vec3(0, 1, 0));
	mat4 P(1.0);

	if (projection_mode == 0) {
		P = parallel(1.2, aspect, 0.01, 10.0);
	}
	else {
		P = perspective(M_PI / 180.0 * (30.0), aspect, 0.01, 10.0);
	}

	glUniformMatrix4fv(2, 1, GL_FALSE, value_ptr(V));
	glUniformMatrix4fv(3, 1, GL_FALSE, value_ptr(P));

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);

	// draw colored object(fill)
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	
	// 0: fragment shader => fcolor
	glUniform1i(4, 0);

	if (projection_mode == 1)
		draw_cube(value_ptr(T));
	else if (projection_mode == 2)
		draw_sphere(value_ptr(T));
	else if (projection_mode == 3)
		draw_cone(vao[projection_mode]);

	glDisable(GL_POLYGON_OFFSET_FILL);

	// if show_wireframes is true
	if (show_wireframes)
	{
		// draw line on sphere
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(0.7);

		// 1: fragment shader => black line
		glUniform1i(4, 1);
	}

	// if show_vertices is true
	if (show_vertices)
	{
		// draw point on sphere
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		glPointSize(2);

		// 1: fragment shader => black point
		glUniform1i(4, 1);
	}

	glFlush();

	// Draw again.
	glutPostRedisplay();
}


void init(GLuint program, GLfloat radius, GLfloat height, GLuint subdivs)
{

	get_cube_3d(vtx_pos[0], 1, 1, 1);
	get_sphere_3d(vtx_pos[1], 0.8f, 20, 10);
	get_cone_3d(vtx_pos[2], idx_list[0], idx_list[1], 0.8, 20, 10);

	for (int i = 0; i < 2; i++) {
		get_color_3d_by_pos(vtx_clrs[i], vtx_pos[i]);
		glGenVertexArrays(1, &vao[i]);
		glBindVertexArray(vao[i]);


		const GLchar* attri_name[2] = { "vPosition", "vColor" };
		GLvec* vtx_list[2] = { &(vtx_pos[i]), &(vtx_clrs[i]) };

		glGenBuffers(2, element_buffs);
		for (int j = 0; j < 2; ++j) {
			bind_buffer(vbo[i][0], vtx_pos[i], program, attri_name[0], 3);
			bind_buffer(vbo[i][1], vtx_clrs[i], program, attri_name[1], 3);
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffs[j]);
			//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(size_t) * idx_list[j].size(),
			//	idx_list[j].data(), GL_STATIC_DRAW);
		}
	}
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

void draw_cube(const GLfloat* trans_mat)
{
	glUniformMatrix4fv(1, 1, GL_FALSE, trans_mat);
	glDrawArrays(GL_TRIANGLES, 0, vtx_pos[0].size() / 3);
}

void get_sphere_3d(GLvec& p, GLfloat r, GLint subh, GLint suba)
{
	for (int i = 1; i <= subh; ++i) {
		double theta0 = M_PI * (i-1) / subh;
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
			double phi0 = 2 * M_PI * (j-1) / suba;
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
			FSET_VTX3(vx0, vy0, vz0, sp0*rst0, y0, cp0*rst0);
			FSET_VTX3(vx1, vy1, vz1, sp0*rst1, y1, cp0*rst1);
			FSET_VTX3(vx2, vy2, vz2, sp1*rst0, y0, cp1*rst0);
			FSET_VTX3(vx3, vy3, vz3, sp1*rst1, y1, cp1*rst1);

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

void draw_sphere(const GLfloat* trans_mat)
{
	// transfer values to uniform variables.
	glUniformMatrix4fv(1, 1, GL_FALSE, trans_mat);
	// draw triangles
	glDrawArrays(GL_TRIANGLES, 0, vtx_pos[1].size() / 3);
}

void get_cone_3d(GLvec& p, GLsvec& side_idx, GLsvec& botton_idx, GLfloat radius, GLfloat height, GLint n)
{
	GLfloat half_height = height / 2;
	GLfloat theta, x, z;

	FPUSH_VTX3(p, 0, half_height, 0);
	side_idx.push_back(0);
	for (int i = 0; i <= n; ++i) {
		theta = (GLfloat)(2.0 * M_PI * i / n);
		x = radius * sin(theta);
		z = radius * cos(theta);
		FPUSH_VTX3(p, x, -half_height, z);
		side_idx.push_back(i + 1);
		botton_idx.push_back(n + 2 - i);
	}
	FPUSH_VTX3(p, 0, -half_height, 0);
	botton_idx.push_back(1);
}

void draw_cone(GLuint vao)
{
	glBindVertexArray(vao);
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

	FPUSH_VTX3_AT(p, 0, 0, half_height, 0);
	top_idx.push_back(0);
	bottom_idx.push_back(2*n+3);
	for (int i = 0; i <= n; ++i) {
		theta = (GLfloat)(2.0 * M_PI * i / n);
		x = radius * sin(theta);
		z = radius * cos(theta);
		FPUSH_VTX3_AT(p, 2 * i + 1, x, half_height, z);
		FPUSH_VTX3_AT(p, 2 * i + 2, x, -half_height, z);
		side_idx.push_back(2 * i + 1);
		side_idx.push_back(2 * i + 2);
		top_idx.push_back(2 * i + 1);
		bottom_idx.push_back(2 * n + 2 - 2 * i);
	}
	FPUSH_VTX3_AT(p, 2 * n + 3, 0, -half_height, 0);
}

void get_tours_3d(GLvec& p, vector<GLsvec>& side_idx, GLfloat r0, GLfloat r1, GLint na, GLint nh)
{

}
