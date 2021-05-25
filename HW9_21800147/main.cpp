#include <stdio.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
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
bool show_axis = false;
bool show_grid = false;

int button_pressed[3] = { GLUT_UP, GLUT_UP, GLUT_UP };
int mouse_pos[2] = { 0, 0 };

glm::mat4 parallel(double r, double aspect, double n, double f)
{
	double l = -r;
	double width = 2 * r;
	double height = width / aspect;
	double t = height / 2;
	double b = -t;

	return glm::ortho(l, r, b, t, n, f);
}

struct Camera
{
	enum { ORTHOGRAPHIC, PERSPECTIVE };
	glm::vec3 eye;
	glm::vec3 center;
	glm::vec3 up;
	float zoom_factor;
	int c_projection_mode;
	float z_near;
	float z_far;
	float fovy;
	float x_right;

	Camera() :
		eye(0, 0, 8),
		center(0, 0, 0),
		up(0, 1, 0),
		zoom_factor(1.0f),
		c_projection_mode(ORTHOGRAPHIC),
		z_near(0.01f),
		z_far(100.0f),
		fovy((float)(M_PI / 180.0 * (30.0))),
		x_right(1.2f)
	{}

	glm::mat4 get_viewing() { return glm::lookAt(eye, center, up); }

	glm::mat4 get_projection(float aspect)
	{
		glm::mat4 P;
		switch(c_projection_mode)
		{
		case ORTHOGRAPHIC:
			P = parallel((double)zoom_factor * x_right, aspect, z_near, z_far);
			break;

		case PERSPECTIVE:
			P = glm::perspective(zoom_factor * fovy, aspect, z_near, z_far);
			break;
		}
		return P;
	}
};

Camera camera;

void mouse(int button, int state, int x, int y)
{
	button_pressed[button] = state;
	mouse_pos[0] = x;
	mouse_pos[1] = y;
}

void motion(int x, int y)
{
	using namespace glm;

	int modifiers = glutGetModifiers();
	int is_alt_active = modifiers & GLUT_ACTIVE_ALT;
	int is_ctrl_active = modifiers & GLUT_ACTIVE_CTRL;
	int is_shift_active = modifiers & GLUT_ACTIVE_SHIFT;

	int w = glutGet(GLUT_WINDOW_WIDTH);
	int h = glutGet(GLUT_WINDOW_HEIGHT);
	GLfloat dx = 1.f * (x - mouse_pos[0]) / w;
	GLfloat dy = -1.f * (y - mouse_pos[1]) / h;

	if (button_pressed[GLUT_LEFT_BUTTON] == GLUT_DOWN)
	{
		if (is_alt_active)
		{
			vec4 disp(camera.eye - camera.center, 1);

			GLfloat alpha = 2.0f;
			mat4 V = camera.get_viewing();
			mat4 Rx = rotate(mat4(), alpha * dy, vec3(transpose(V)[0]));
			mat4 Ry = rotate(mat4(), -alpha * dx, vec3(0, 1, 0));
			mat4 R = Ry * Rx;
			camera.eye = camera.center + vec3(R * disp);
			camera.up = mat3(R) * camera.up;

		}
	}

	if (button_pressed[GLUT_MIDDLE_BUTTON] == GLUT_DOWN)
	{
		if (is_alt_active) {
			mat4 VT = transpose(camera.get_viewing());
			camera.eye += vec3(-dx * VT[0] + -dy * VT[1]);
			camera.center += vec3(-dx * VT[0] + -dy * VT[1]);
		}
	}

	mouse_pos[0] = x;
	mouse_pos[1] = y;
	glutPostRedisplay();

}

void mouse_wheel(int wheel, int dir, int x, int y)
{
	int is_alt_active = glutGetModifiers() & GLUT_ACTIVE_ALT;

	if (is_alt_active) {
		glm::vec3 disp = camera.eye - camera.center;
		if (dir > 0)
			camera.eye = camera.center + 0.95f * disp;
		else
			camera.eye = camera.center + 1.05f * disp;
	}
	else {
		if (dir > 0)
			camera.zoom_factor *= 0.95;
		else
			camera.zoom_factor *= 1.05f;
	}

	glutPostRedisplay();
}

void cb_menu(int value)
{
	projection_mode = value;
}

void init()
{
	// 마우스 함수를 따로 사용하지 않고 이렇게만 사용해도 가능한가?
	int menu_id = glutCreateMenu(cb_menu);
	glutAddMenuEntry("Orthographic projection", 0);
	glutAddMenuEntry("Perspective projection", 1);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	models.push_back(new CubePrimitive(1.0f, 1.0f, 1.0f));
	models.push_back(new SpherePrimitive(0.5f, 15, 15));
	models.push_back(new ConePrimitive(0.5f, 1.0f, 10));
	models.push_back(new CylinderPrimitive(0.5f, 1.0f, 15));
	models.push_back(new TorusPrimitive(0.3f, 0.3f, 30, 10));
	models.push_back(new CarModel(
		(CubePrimitive*) models[0],
		(SpherePrimitive*) models[1],
		(ConePrimitive*) models[2],
		(CylinderPrimitive*) models[3],
		(TorusPrimitive*) models[4]));
	models.push_back(new Grid(3.0f, 3.0f, 10, 10));
	models.push_back(new CylinderPrimitive(0.02f, 1.0f, 10)); // R: x-axis
	models.push_back(new CylinderPrimitive(0.02f, 1.0f, 10)); // G: y-axis
	models.push_back(new CylinderPrimitive(0.02f, 1.0f, 10)); // B: z-axis

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

void display()
{
	using namespace glm;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);
	glUniform1i(4, 0);

	mat4 Transf(1.0f);
	GLfloat theta = 0.6f;
	Transf = rotate(Transf, theta, vec3(1.5f, 0.0f, 0.0f));
	Transf = rotate(Transf, theta, vec3(0.0f, -1.2f, 0.0f));
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

	if (show_grid)
	{
		if (show_axis) {
			Transf = translate(Transf, vec3(0.0f, 0.5f, 0.0f));
			glUniformMatrix4fv(1, 1, GL_FALSE, value_ptr(Transf));
		}
		glLineWidth(1.5f);
		glUniform1i(4, 2);
		models[6]->draw();
	}

	if (show_axis)
	{
		mat4 T(1.0f);
		T = translate(T, vec3(0.0f, 0.5f, 0.0f));
		glUniformMatrix4fv(1, 1, GL_FALSE, value_ptr(T));
		glUniform1i(4, 3);
		models[7]->draw();
		glUniform1i(4, 4);
		models[8]->draw();
		glUniform1i(4, 5);
		models[9]->draw();

	}

	if (show_wireframe)
	{
		glLineWidth(1.0f);
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
	case 'o':
		projection_mode = (projection_mode + 1) % 2;
		break;

	case 'a':
		show_axis = !show_axis;
		break;

	case 'g':
		show_grid = !show_grid;
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
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutMouseWheelFunc(mouse_wheel);
	glutIdleFunc(idle);
	glutMainLoop();
	release_models();
}