#include <stdio.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "LoadShaders.h"
#define _USE_MATH_DEFINES // required to use M_PI
#include <math.h>
#include <vector>

typedef std::vector<GLfloat> GLvec;

GLuint VertexArrays[3];
GLuint Buffers[3][2];
GLuint active_vao = 0; //index for the active vertex array object
GLvec vertices[2]; // 2d vertex list
GLvec colors[3]; // vertex color (r,g,b) list

void keyboard(unsigned char key, int x, int y);
void display();
void init();
int build_program();
void get_circle_2d(GLvec& p, int sectors, GLfloat radius);
void get_rect_2d(GLvec& p, GLfloat width, GLfloat height);
void get_vertex_color(GLvec& color, GLuint n, GLfloat r, GLfloat g, GLfloat b);
void bind_buffer(GLuint buffer, GLvec& vec, int program,
	const GLchar* attri_name, GLint attri_size);
void share_buffer(GLuint buffer, int program,
	const GLchar* attri_name, GLint attri_size);

void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(512, 512);
	glutCreateWindow("VAO example");
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		exit(EXIT_FAILURE);
	}
	init();
	glutDisplayFunc(display);
	// keyboard callback function in GLUT
	glutKeyboardFunc(keyboard);
	glutMainLoop();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
		// Let the GLUT redraw the screen by calling display. (the correct number for keyboard.)
		case '1': active_vao = 0; glutPostRedisplay(); break;
		case '2': active_vao = 1; glutPostRedisplay(); break;
		case '3': active_vao = 2; glutPostRedisplay(); break;
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(VertexArrays[active_vao]);
	// if active_vao == 1, make rectangle
	if(active_vao == 1)
		glDrawArrays(GL_TRIANGLES, 0, vertices[active_vao].size() / 2);
	// else, make circle (same vertices - share)
	else
		glDrawArrays(GL_TRIANGLE_FAN, 0, vertices[0].size() / 2);
	glFlush();
}

void init()
{
	int program = build_program();
	get_circle_2d(vertices[0], 50, 0.5f);
	get_rect_2d(vertices[1], 1.0f, 0.8f);
	get_vertex_color(colors[0], vertices[0].size() / 2, 0.8f, 0.2f, 0.5f);
	get_vertex_color(colors[1], vertices[1].size() / 2, 0.9f, 0.9f, 0.1f);
	// add blue color
	get_vertex_color(colors[2], vertices[0].size() / 2, 0.3f, 0.3f, 1.0f);

	glGenVertexArrays(3, VertexArrays);
	for (int i = 0; i < 2; ++i) {
		glBindVertexArray(VertexArrays[i]);
		glGenBuffers(2, Buffers[i]);
		bind_buffer(Buffers[i][0], vertices[i], program, "vPosition", 2);
		bind_buffer(Buffers[i][1], colors[i], program, "vColor", 3);
	}

    // share buffer
	glBindVertexArray(VertexArrays[2]);
	share_buffer(Buffers[0][0], program, "vPosition", 2);
    // generate color buffer
	glGenBuffers(1, Buffers[2]);
	bind_buffer(Buffers[2][0], colors[2], program, "vColor", 3);
}

int build_program()
{
	ShaderInfo shaders[] = {
		{GL_VERTEX_SHADER, "vao_example.vert"},
		{GL_FRAGMENT_SHADER, "vao_example.frag"},
		{GL_NONE, NULL}
	};
	GLuint program = LoadShaders(shaders);
	glUseProgram(program);
	return program;
}

#define VSET2(v,a,b) do {(v)[0]=(a); (v)[1]=(b);} while(0)
#define VSET2PP(v,a,b) do{VSET2(v,a,b); v+=2;} while(0)

void get_circle_2d(GLvec& p, int sectors, GLfloat radius)
{
	p.resize((sectors + 2) * 2);
	GLfloat* data = p.data();

	// Push the center of a circle.
	VSET2PP(data, 0, 0);

	// Push all circular points that constructs the circle.
	for (int i = 0; i <= sectors; ++i) {
		GLfloat phi = (GLfloat)(2 * M_PI * i / sectors);
		GLfloat x = radius * cos(phi);
		GLfloat y = radius * sin(phi);
		VSET2PP(data, x, y);
	}
}

void get_rect_2d(GLvec& p, GLfloat width, GLfloat height)
{
	GLfloat w2 = width / 2;
	GLfloat h2 = height / 2;

	p.resize(12);
	GLfloat* data = p.data();

	// first triangle
	VSET2PP(data, -w2, -h2);
	VSET2PP(data, +w2, -h2);
	VSET2PP(data, -w2, +h2);

	// second triangle
	VSET2PP(data, +w2, -h2);
	VSET2PP(data, +w2, +h2);
	VSET2PP(data, -w2, +h2);
}

void get_vertex_color(GLvec& color, GLuint n, GLfloat r, GLfloat g, GLfloat b)
{
	color.resize(n * 3);
	for (GLuint i = 0; i < n; ++i) {
		color[i * 3 + 0] = r;
		color[i * 3 + 1] = g;
		color[i * 3 + 2] = b;
	}
}

void bind_buffer(GLuint buffer, GLvec& vec, int program,
	const GLchar* attri_name, GLint attri_size)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	// Put the data in the actual buffer.
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vec.size(),
		vec.data(), GL_STATIC_DRAW);
	GLint location = glGetAttribLocation(program, attri_name);
	glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(location);
}

void share_buffer(GLuint buffer, int program,
	const GLchar* attri_name, GLint attri_size)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	GLint location = glGetAttribLocation(program, attri_name);
	glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(location);
}

