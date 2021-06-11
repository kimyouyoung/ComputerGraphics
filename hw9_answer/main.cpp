#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "LoadShaders.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "primi.h"
#include "camera.h"

using namespace std;

CubePrimitive cube(1.0f, 1.0f, 1.0f);
SpherePrimitive sphere(0.5f, 30, 30);
ConePrimitive cone(0.5f, 1.0f, 10);
CylinderPrimitive cylinder(0.5f, 1.0f, 30);
TorusPrimitive torus(0.3f, 0.3f, 30, 10);
Grid grid(5, 5, 10, 10);
CarModel car(&cube, &sphere, &cone, &cylinder, &torus);
vector<Model*> models;

GLuint program;
int idx_selected = 0;

bool show_wireframe = false;
bool show_axis = false;
bool show_grid = false;

int button_pressed[3] = { GLUT_UP, GLUT_UP, GLUT_UP };
int mouse_pos[2] = { 0, 0 };

Camera camera;
struct ModelState {
	glm::vec3 pos;
	glm::vec3 scale;
	GLfloat theta;

	ModelState() : pos(0), scale(1), theta(0){}
} model_state;

void set_color(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0f)
{
	using namespace glm;
	glUniform1i(2, 1);
	glUniform4fv(3, 1, value_ptr(vec4(r, g, b, a)));
}

void cb_main_menu(int value)
{
	switch (value)
	{
	case 0:
		camera.c_projection_mode = Camera::ORTHOGRAPHIC;
		break;
	case 1:
		camera.c_projection_mode = Camera::PERSPECTIVE;
		break;
	}
	glutPostRedisplay();
}

void init()
{
	int menu_id = glutCreateMenu(cb_main_menu);
	glutAddMenuEntry("Orthographic projection", 0);
	glutAddMenuEntry("Perspective projection", 1);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	models.push_back(&cube);
	models.push_back(&sphere);
	models.push_back(&cone);
	models.push_back(&cylinder);
	models.push_back(&torus);
	models.push_back(&car);
	models.push_back(&grid);

	ShaderInfo shaders[] = {
		{GL_VERTEX_SHADER, "viewing.vert"},
		{GL_FRAGMENT_SHADER, "viewing.frag"},
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
}

void draw_cylinder(glm::mat4& T_in, GLfloat radius, GLfloat height,
	GLfloat dx = 0, GLfloat dy = 0, GLfloat dz = 0,
	glm::mat4* R0 = NULL)
{
	using namespace glm;
	mat4 T = T_in;
	T = translate(T, vec3(dx, dy, dz));
	if (R0) T *= (*R0);
	T = scale(T, vec3(2 * radius, height, 2 * radius));
	GLint location = glGetUniformLocation(program, "M");
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(T));
	glUniformMatrix4fv(1, 1, GL_FALSE, value_ptr(mat4(1.0f)));

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	cylinder.draw();
}

void draw_coordinate_system(glm::mat4& T)
{
	using namespace glm;
	set_color(0, 1, 0);
	draw_cylinder(T, 0.02f, 1.0f, 0, 0.5f, 0);

	set_color(0, 0, 1);
	draw_cylinder(T, 0.02f, 1.0f, 0, 0, 0.5f,
		&rotate(mat4(1.0f), radians(90.0f), vec3(1, 0, 0)));

	set_color(1, 0, 0);
	draw_cylinder(T, 0.02f, 1.0f, 0.5f, 0, 0,
		&rotate(mat4(1.0f), radians(90.0f), vec3(0, 0, 1)));
}

void display()
{
	using namespace glm;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);
	glUniform1i(2, 0);

	GLint location;

	// modeling transformation matrix
	mat4 T(1.0f);
	glUniformMatrix4fv(1, 1, GL_FALSE, value_ptr(T));

	// modeling transformation matrix
	mat4 M(1.0f);
	M = translate(M, model_state.pos);
	M = rotate(M, model_state.theta, vec3(0.0f, 1.0f, 0.0f));
	M = scale(M, model_state.scale);
	location = glGetUniformLocation(program, "M");
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(M));
	
	// viewing transformation matrix
	mat4 V = camera.get_viewing();
	location = glGetUniformLocation(program, "V");
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(V));

	// projection transformation
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);
	double aspect = 1.0 * width / height;

	mat4 P = camera.get_projection(aspect);
	location = glGetUniformLocation(program, "P");
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(P));

	// Draw the selected model
	models[idx_selected]->draw();
	
	glDisable(GL_POLYGON_OFFSET_FILL);

	if (show_wireframe)
	{
		set_color(0, 0, 0);
		glLineWidth(1.0f);
		glUniform1i(2, 1);
		models[idx_selected]->draw_wire();
	}

	if (show_axis)
	{
		draw_coordinate_system(M);
	}

	// draw the grid
	if (show_grid)
	{
		set_color(1, 1, 1);
		glLineWidth(1.5f);
		glUniform1i(2, 0);
		location = glGetUniformLocation(program, "T");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(mat4(1.0f)));
		location = glGetUniformLocation(program, "M");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(mat4(1.0f)));
		grid.draw();
	}

	glFlush();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		show_wireframe = !show_wireframe;
		glutPostRedisplay();
		break;

	case 'a':
		show_axis = !show_axis;
		glutPostRedisplay();
		break;

	case 'g':
		show_grid = !show_grid;
		glutPostRedisplay();
		break;

	case 'e':
		glutLeaveMainLoop();
		break;

	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
		idx_selected = key - '1';
		glutPostRedisplay();
		break;
	}
}

void specialkey(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_HOME:
		model_state.theta += 0.005f;
		car.turn_left();
		glutPostRedisplay();
		break;

	case GLUT_KEY_END:
		model_state.theta -= 0.005f;
		car.turn_right();
		glutPostRedisplay();
		break;

	case GLUT_KEY_LEFT:
		using namespace glm;
		mat4 R1 = rotate(mat4(1.0f), model_state.theta, vec3(0.0f, 1.0f, 0.0f));
		model_state.pos -= 0.005f * vec3(R1[0]);
		car.move_forward();
		glutPostRedisplay();
		break;

	case GLUT_KEY_RIGHT:
		using namespace glm;
		mat4 R2 = rotate(mat4(1.0f), model_state.theta, vec3(0.0f, 1.0f, 0.0f));
		model_state.pos += 0.005f * vec3(R2[0]);
		car.move_backward();
		glutPostRedisplay();
		break;

	case GLUT_KEY_UP:
		model_state.pos[1] += 0.01f;
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		model_state.pos[1] -= 0.01f;
		glutPostRedisplay();
		break;

	}
}

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

	// for ALT key
	int is_alt_active = modifiers & GLUT_ACTIVE_ALT;
	// for Control Key
	int is_ctrl_active = modifiers & GLUT_ACTIVE_CTRL;
	// for Shith Key
	int is_shift_active = modifiers & GLUT_ACTIVE_SHIFT;

	int w = glutGet(GLUT_WINDOW_WIDTH);
	int h = glutGet(GLUT_WINDOW_HEIGHT);
	GLfloat dx = 1.f * (x - mouse_pos[0]) / w;
	GLfloat dy = -1.f * (y - mouse_pos[1]) / h;

	bool redraw = false;

	// tumble tool
	if (button_pressed[GLUT_LEFT_BUTTON] == GLUT_DOWN)
	{
		if (is_alt_active)
		{
			vec4 disp(camera.eye - camera.center, 1);

			GLfloat alpha = 2.0f;
			mat4 V = camera.get_viewing();
			mat4 Rx = rotate(mat4(1.0f), alpha * dy, vec3(transpose(V)[0]));
			mat4 Ry = rotate(mat4(1.0f), -alpha * dx, vec3(0, 1, 0));
			mat4 R = Ry * Rx;
			camera.eye = camera.center + vec3(R * disp);
			camera.up = mat3(R) * camera.up;
			redraw = true;
		}
	}

	// track tool
	if (button_pressed[GLUT_MIDDLE_BUTTON] == GLUT_DOWN)
	{
		if (is_alt_active) {
			mat4 VT = transpose(camera.get_viewing());
			camera.eye += vec3(-dx * VT[0] + -dy * VT[1]);
			camera.center += vec3(-dx * VT[0] + -dy * VT[1]);
			redraw = true;
		}
	}

	mouse_pos[0] = x;
	mouse_pos[1] = y;

	if(redraw)
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

void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
	glutInitWindowSize(512, 512);
	glutCreateWindow("Camera Control");
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialkey);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutMouseWheelFunc(mouse_wheel);
	glutMainLoop();
}