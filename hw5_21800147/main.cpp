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
#include <fstream>
#include <string>
#include <string.h>
using namespace std;

using namespace glm;
typedef std::vector<GLfloat> GLvec;

const GLuint num_of_models = 3;

const char* obj_filepath[num_of_models] = {
	"house.obj",
	"sphere.obj",
	"teapot.obj",
};

GLvec vertices[num_of_models];
std::vector<GLuint> faces[num_of_models];

GLuint vao[num_of_models];
GLuint vbo[num_of_models][2];

int program;
GLuint active_vao = 0;

void keyboard(unsigned char key, int x, int y);
void display();
void init();
int build_program();
void bind_buffer(GLuint buffer, GLvec& vec, int program,
	const GLchar* attri_name, GLint attri_size);
void bind_buffer(GLint buffer, std::vector<GLuint>& vec);
bool load_obj(GLvec& vertices, std::vector<GLuint>& faces, const char* filepath);

void main(int argc, char** argv)
{
	// initialize GLUT to make a window
	glutInit(&argc, argv);
	// initialize and set the basic window
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(512, 512);
	glutCreateWindow("Simple OBJ");

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
		// assign to a according to the number received on the keyboard. (1: house, 2: sphere, 3: teapot)
		case '1': active_vao = 0; glutPostRedisplay(); break;
		case '2': active_vao = 1; glutPostRedisplay(); break;
		case '3': active_vao = 2; glutPostRedisplay(); break;

	}
}

void display()
{
	// clear the color buffer, the depth buffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// bind vertex buffers to be referenced by vao.
	glBindVertexArray(vao[active_vao]);

	GLint location;
	// multiply 0.001f and time to set the angle.
	GLfloat theta = 0.001f * clock();
	// returns the index of a general vertex attribute bound to that attribute variable.
	location = glGetUniformLocation(program, "active_vao");
	// set the value of uniform.
	glUniform1i(location, active_vao);

	mat4 T(1.0f);
	// rotate object Y-axis
	T = rotate(T, theta, vec3(0.0f, 0.1f, 0.0f));

	// returns the index of a general vertex attribute bound to that attribute variable.
	location = glGetUniformLocation(program, "T");
	// transfer values to uniform variables.
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(T));

	// use the glDrawElements function to draw in index order.
	glDrawElements(GL_TRIANGLES, faces[active_vao].size(), GL_UNSIGNED_INT, (void*)0);
	glFlush();

	// Draw again.
	glutPostRedisplay();
}


void init()
{
	srand(clock());
	program = build_program();

	for (int i = 0; i < num_of_models; ++i) {
		// load object files.
		load_obj(vertices[i], faces[i], obj_filepath[i]);

		// generate a new vertex array object
		glGenVertexArrays(1, &vao[i]);
		// initialize vertex buffers to be referenced by vao.
		glBindVertexArray(vao[i]);
		// generate a new buffer objects.
		glGenBuffers(2, vbo[i]);

		bind_buffer(vbo[i][0], vertices[i], program, "vPosition", 3);
		bind_buffer(vbo[i][1], faces[i]);
	}

	// active depth test
	glEnable(GL_DEPTH_TEST);
	// pass if the depth value of the frame is less than the stored depth value.
	glDepthFunc(GL_LESS);

	// set the thickness of the line to 1.0f.
	glLineWidth(1.0f);
	// set to draw lines only on both the front and back sides.
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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

void bind_buffer(GLint buffer, std::vector<GLuint>& vec)
{
	// bind the buffer object to GL_ELEMENT_ARRAY_BUFFER type for faces.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * vec.size(), vec.data(), GL_STATIC_DRAW);
}

bool load_obj(GLvec& vertices, std::vector<GLuint>& faces, const char* filepath)
{
	std::ifstream file;
	// open the object file
	file.open(filepath);

	float value;
	int face;

	// if the file was opened normally, run the code below.
	if (file.is_open())
	{
		// repeat to the end of the file.
		while (!file.eof())
		{
			char arr[256];
			// read from file one line at a time
			file.getline(arr, 256);

			// if it starts with a "v", put it in vertices vector(but ignore it if it's a "vt").
			if (arr[0] == 'v' && arr[1] != 't')
			{
				char* context = NULL;
				// cut strings based on spacing.
				char* tok = strtok_s(arr, " ", &context);
				while (tok) {
					tok = strtok_s(NULL, " ", &context);
					// if tok is not nullptr
					if (tok != nullptr) {
						// convert tok(string) to float type and put it in the vertices vector.
						value = stof(tok);
						vertices.push_back(value);
					}
				}
			}

			if (arr[0] == 'f')
			{
				char* context2 = NULL;
				// cut strings based on spacing.
				char* tok2 = strtok_s(arr, " ", &context2);
				while (tok2) {
					tok2 = strtok_s(NULL, " ", &context2);
					// if tok2 is not nullptr
					if (tok2 != nullptr) {
						// convert tok2(string) to int type and put it in the faces vector
						face = stoi(tok2);
						faces.push_back(face-1);
					}
				}
			}
		}
		return true;
	}
	// If the file does not open properly, return false.
	else
		return false;
}