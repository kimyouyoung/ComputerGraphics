#include <stdio.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "LoadShaders.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <time.h>
#include "primi.h"
#include <vector>
#define _USE_MATH_DEFINES // required to use M_PI
#include <math.h>

using namespace std;

GLuint program;
int idx_selected = 0;
int projection_mode = 0;
vector<Model*> models;

bool show_wireframe = false;
bool show_points = false;

void init()
{
	models.push_back(new CubePrimitive(1.0f, 1.0f, 1.0f));
	models.push_back(new SpherePrimitive(0.5f, 15, 15));
	models.push_back(new ConePrimitive(0.5f, 1.0f, 10));
	models.push_back(new CylinderPrimitive(0.5f, 1.0f, 15));
	models.push_back(new TorusPrimitive(0.3f, 0.3f, 30, 10));
	models.push_back(new CarModel(
		(CubePrimitive*)models[0],
		(SpherePrimitive*)models[1],
		(ConePrimitive*)models[2],
		(CylinderPrimitive*) models[3],
		(TorusPrimitive*) models[4]));

	ShaderInfo shaders[] = {
		{GL_VERTEX_SHADER, "vao_example.vert"},
		{GL_FRAGMENT_SHADER, "vao_example.frag"},
		{GL_NONE, NULL}
	};

	program = LoadShaders(shaders);
	glUseProgram(program);

	int num_of_models = (int)models.size();
	for (int i = 0; i < num_of_models; ++i) {
		models[i]->init(program);
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
}

void release_models()
{
	int n = (int)models.size();
	for (int i = 0; i < n; ++i) {
		if (models[i]) {
			delete models[i];
			models[i] = NULL;
		}
	}
	models.clear();
}

void idle()
{
	static clock_t prev_time = clock();
	clock_t curr_time = clock();
	if (1.0 * (curr_time - prev_time) / CLOCKS_PER_SEC > 0.005) {
		glutPostRedisplay();
		prev_time = curr_time;
	}
}

glm::mat4 parallel(double r, double aspect, double n, double f)
{
	double l = -r;
	double width = 2 * r;
	double height = width / aspect;
	double t = height / 2;
	double b = -t;

	return glm::ortho(l, r, b, t, n, f);
}

void display()
{
	using namespace glm;

	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);
	glUniform1i(4, 0);

	mat4 Transf(1.0f);
	GLfloat theta = 0.001f * clock();
	Transf = rotate(Transf, theta, vec3(-1.0f, 1.0f, 0.0f));
	Transf = scale(Transf, vec3(1.0f));
	glUniformMatrix4fv(1, 1, GL_FALSE, value_ptr(Transf));

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

	models[idx_selected]->draw();

	glDisable(GL_POLYGON_OFFSET_FILL);

	if (show_wireframe)
	{
		glLineWidth(1.5f);
		glUniform1i(4, 1);
		models[idx_selected]->draw_wire();
	}

	if (show_points)
	{
		glPointSize(4.0f);
		glUniform1i(4, 1);
		models[idx_selected]->draw_points();
	}

	glFlush();
	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		show_wireframe = !show_wireframe;
		break;

	case 'v':
		show_points = !show_points;
		break;

	case 'p': 
		projection_mode = (projection_mode + 1) % 2;
		break;

	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
		idx_selected = key - '1';
		break;
	}

}

void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(512, 512);
	glutCreateWindow("Drawing Primitives");
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);
	glutMainLoop();
	release_models();
}