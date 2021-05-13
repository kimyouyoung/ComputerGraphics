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

const GLuint num_of_model = 3;

GLvec vertices[num_of_model];
GLvec colors[num_of_model];

GLuint vao[num_of_model];
GLuint vbo[num_of_model][2];

int program;
bool show_vertices = false;
bool show_wireframes = false;

void keyboard(unsigned char key, int x, int y);
void display();
void init();
int build_program();
void bind_buffer(GLuint buffer, GLvec& vec, int program,
	const GLchar* attri_name, GLint attri_size);
void get_sphere_3d(GLvec& p, GLfloat r, GLint subh, GLint suba);
void get_color_3d_by_pos(GLvec& c, GLvec& p, GLfloat offset);
void draw_sphere(const GLfloat* trans_mat);

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

void main(int argc, char** argv)
{
	// initialize GLUT to make a window
	glutInit(&argc, argv);
	// initialize and set the basic window
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(512, 512);
	glutCreateWindow("Solar System");

	// initialize GLEW to load OpenGL extensions
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	init();
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
		// assign to a according to the number received on the keyboard. (v: vertices, w: wireframes)
		case 'v': show_vertices = true; show_wireframes = false; glutPostRedisplay(); break;
		case 'w': show_wireframes = true; show_vertices = false; glutPostRedisplay(); break;
		default: show_wireframes = false; show_vertices = false; glutPostRedisplay(); break;
	}
}

void draw_sphere(const GLfloat* trans_mat)
{
	// transfer values to uniform variables.
	glUniformMatrix4fv(1, 1, GL_FALSE, trans_mat);
	// draw triangles
	glDrawArrays(GL_TRIANGLES, 0, vertices[0].size()/3);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// multiply 0.001f and time to set the angle.
	GLfloat theta = 0.001f * clock();
	// multiply 0.003f and time to set the angle. (for moon: more faster than earth and sun)
	GLfloat theta2 = 0.003f * clock();

	// bind vertex buffers to be referenced by vao.(each sun, earth, and moon vao)
	for (int i = 0; i < num_of_model; ++i)
		glBindVertexArray(vao[i]);

	// COMPUTE TRANSFORMATIONS each sun, earth and moon
	mat4 T_sun(1.0f);
	// sun: self-rotation => Y-axis, origin
	T_sun = rotate(T_sun, theta, vec3(0.0f, 0.1f, 0.0f));
	T_sun = scale(T_sun, vec3(0.5f));

	mat4 T_earth(1.0f);
	// orbiting around the sun
	T_earth = rotate(T_earth, theta, vec3(0.0f, 0.1f, 0.0f));
	// translation to the earth position
	T_earth = translate(T_earth, vec3(0.8f, 0.0f, 0.0f));
	// earth self-rotation
	T_earth = rotate(T_earth, theta, vec3(0.0f, 0.1f, 0.0f));
	// scale: more smaller than sun
	T_earth = scale(T_earth, vec3(0.12f));

	mat4 T_moon(1.0f);
	// orbiting around the sun
	T_moon = rotate(T_moon, theta, vec3(0.0f, 0.1f, 0.0f));
	// same position with earth
	T_moon = translate(T_moon, vec3(0.8f, 0.0f, 0.0f));
	// orbiting around the earth
	T_moon = rotate(T_moon, theta2, vec3(0.0f, 0.1f, 0.0f));
	// translation to the moon position
	T_moon = translate(T_moon, vec3(0.2f, 0.0f, 0.0f));
	// moon self-rotation
	T_moon = rotate(T_moon, theta2, vec3(0.0f, 0.1f, 0.0f));
	// scale: more smaller than earth
	T_moon = scale(T_moon, vec3(0.08f));


	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);

	// draw colored object(fill)
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// 0: fragment shader => fcolor
	glUniform1i(2, 0);
	// draw each sun, earth, moon
	draw_sphere(value_ptr(T_sun));
	draw_sphere(value_ptr(T_earth));
	draw_sphere(value_ptr(T_moon));

	glDisable(GL_POLYGON_OFFSET_FILL);

	// if show_wireframes is true
	if (show_wireframes)
	{
		// draw line on sphere
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(0.7);

		// 1: fragment shader => black line
		glUniform1i(2, 1);
		draw_sphere(value_ptr(T_sun));
		draw_sphere(value_ptr(T_earth));
		draw_sphere(value_ptr(T_moon));
	}

	// if show_vertices is true
	if (show_vertices)
	{
		// draw point on sphere
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		glPointSize(2);

		// 1: fragment shader => black point
		glUniform1i(2, 1);
		draw_sphere(value_ptr(T_sun));
		draw_sphere(value_ptr(T_earth));
		draw_sphere(value_ptr(T_moon));
	}

	glFlush();

	// Draw again.
	glutPostRedisplay();
}


void init()
{
	srand(clock());
	program = build_program();

	for (int i = 0; i < num_of_model; ++i) {
		// make 3d sphere(radius: 0.8f, subdivition height: 20, subdivision axis: 10)
		get_sphere_3d(vertices[i], 0.8f, 20, 10);
		// make object colors using vertex positons
		get_color_3d_by_pos(colors[i], vertices[i], 0);

		// generate a new vertex array object
		glGenVertexArrays(1, &vao[i]);
		// initialize vertex buffers to be referenced by vao.
		glBindVertexArray(vao[i]);
		// generate a new buffer objects.
		glGenBuffers(2, vbo[i]);

		bind_buffer(vbo[i][0], vertices[i], program, "vPosition", 3);
		bind_buffer(vbo[i][1], colors[i], program, "vColor", 3);
	}

	// active depth test
	glEnable(GL_DEPTH_TEST);
	// pass if the depth value of the frame is less than the stored depth value.
	glDepthFunc(GL_LESS);

	// set to only on both the front and back sides.
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_CULL_FACE);
	// remove the rear face.
	glCullFace(GL_BACK);
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
			c[k] = fminf((p[k] - min_val[j]) / width[j] + offset, 1.0f);
		}
	}
}
