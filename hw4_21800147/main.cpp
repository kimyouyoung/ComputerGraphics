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

using namespace glm;
typedef std::vector<GLfloat> GLvec;

int program;
GLuint vao, vbo[2];
GLvec vertices, colors;
GLfloat scaling_factor = 1.0f;
GLuint drawing_mode = 0;

void keyboard(unsigned char key, int x, int y);
void display();
void init();
int build_program();
void get_box_3d(std::vector<GLfloat>& p, GLfloat lx, GLfloat ly, GLfloat lz);
void get_box_random_color(std::vector<GLfloat>& color);
void bind_buffer(GLuint buffer, GLvec& vec, int program,
	const GLchar* attri_name, GLint attri_size);

void main(int argc, char** argv)
{
	// initialize GLUT to make a window
	glutInit(&argc, argv);
	// initialize and set the basic window
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(512, 512);
	glutCreateWindow("Rotating Cube");

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
		// assign to a according to the number received on the keyboard. (1: original, 2: grayscale, 3: complementary)
		case '1': drawing_mode = 0; glutPostRedisplay(); break;
		case '2': drawing_mode = 1; glutPostRedisplay(); break;
		case '3': drawing_mode = 2; glutPostRedisplay(); break;
		
		// adjust the size of the cube. (+: scale up, -: scale down)
		case '+': scaling_factor += 0.1f; break;
		case '-': scaling_factor -= 0.1f; break;
	}
}

void display()
{
	// clear the color buffer, the depth buffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// bind vertex buffers to be referenced by vao.
	glBindVertexArray(vao);

	GLint location;
	// multiply 0.001f and time to set the angle.
	GLfloat theta = 0.001f * clock();
	// returns the index of a general vertex attribute bound to that attribute variable.
	location = glGetUniformLocation(program, "drawing_mode");
	// set the value of uniform.
	glUniform1i(location, drawing_mode);

	mat4 T(1.0f);
	// rotate cube
	T = rotate(T, theta, vec3(1.0f, 0.0f, 0.0f));
	T = rotate(T, theta, vec3(0.0f, 0.1f, 0.0f));
	T = rotate(T, theta, vec3(1.0f, 0.0f, 0.0f));
	// change scale of cube
	T = scale(T, vec3(scaling_factor));

	// returns the index of a general vertex attribute bound to that attribute variable.
	location = glGetUniformLocation(program, "T");
	// transfer values to uniform variables.
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(T));

	// draw two triangles.
	glDrawArrays(GL_TRIANGLES, 0, vertices.size()/3);
	glFlush();
	// Draw again.
	glutPostRedisplay();
}


void init()
{
	srand(clock());
	program = build_program();
	
	get_box_3d(vertices, 1, 1, 1);
	get_box_random_color(colors);
	
	// generate a new vertex array object
	glGenVertexArrays(1, &vao);
	// initialize vertex buffers to be referenced by vao.
	glBindVertexArray(vao);

	// generate a new buffer objects.
	glGenBuffers(2, vbo);

	bind_buffer(vbo[0], vertices, program, "vPosition", 3);
	bind_buffer(vbo[1], colors, program, "vColor", 3);

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

void get_box_3d(std::vector<GLfloat>& p, GLfloat lx, GLfloat ly, GLfloat lz)
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
	size_t n = p.size()/3;
	for(int i = 0;i < n; ++i)
	{
		p[3 * i + 0] *= lx;
		p[3 * i + 1] *= ly;
		p[3 * i + 2] *= lz;
	}
}

void get_box_random_color(std::vector<GLfloat>& color)
{
	color.resize(108); // 6 sides * 2 triangles/side * 3 vertices/triangle * 3 coordinates/vertex = 108 coordinatnes

	// assign the contents of the "color" to "*p".
	GLfloat* p = color.data();

	// repeat for 6 sides
	for(size_t i = 0; i < 6; ++i)
	{
		// put to "side_color" random colors to each of r, g, and b.
		GLfloat side_color[3] = {
			0.5f * rand() / RAND_MAX + 0.5f,
			0.5f * rand() / RAND_MAX + 0.5f,
			0.5f * rand() / RAND_MAX + 0.5f
		};

		for(size_t j = 0; j < 6; ++j)
		{
			*p++ = side_color[0];	// r of vertex j of side i
			*p++ = side_color[1];	// g of vertex j of side i
			*p++ = side_color[2];	// b of vertex j of side i
		}
	}
}

