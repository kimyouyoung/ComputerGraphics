#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "LoadShaders.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "primi.h"
#include "camera.h"
#include "coordinate.h"
#include <time.h>
#include <algorithm>

/*
* 4: N shape  x(0, 2) y(0, 2) z(0, 1)
* xÃà -> 1: y(0, 1) z(0, 2), 2: y(-1, 1) z(0, 1), 3: y(0, 1) z(-1, 1)
* yÃà -> 1: x(0, 1) z(-1, 1), 2: x(-1, 1) z(0, 1), 3: x(0, 1) z(0, 2)
* zÃà -> 1: x(-1, 1) y(0, 2), 2: x(-1, 1) y(-1, 1), 3: x(0, 2) y(-1, 1)
*/

using namespace std;

CubePrimitive cube(1.0f, 1.0f, 1.0f);
Grid grid_down(5, 5, 5, 5, 0);
Grid grid_up(5, 5, 5, 5, 10);

Coordinate coor;
LshapeModel LShape(&cube, &coor);
IshapeModel IShape(&cube, &coor);

NshapeModel NShape(&cube, &coor);

BoxshapeModel BoxShape(&cube, &coor);
vector<Model*> models;

GLuint program;
int idx_selected = 3;

GLfloat box_cell = 0.3f / 1;
GLfloat min_y = -1.0f;

int button_pressed[3] = { GLUT_UP, GLUT_UP, GLUT_UP };
int mouse_pos[2] = { 0, 0 };

Camera camera;

struct ModelState {
	glm::vec3 pos;
	glm::vec3 scale;
	glm::mat4 R;

	ModelState() : pos(0.0f, -1.0f + (box_cell * 10) + (box_cell / 2), 0.0f), scale(0.3f), R(1.0f) {}
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
	// 1 - 4 : shapes of blocks
	models.push_back(&LShape);
	models.push_back(&IShape);
	models.push_back(&BoxShape);
	models.push_back(&NShape);

	models.push_back(&grid_down);
	models.push_back(&grid_up);

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

void display()
{
	using namespace glm;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);
	glUniform1i(2, 0);

	printf("x: %d %d\n", coor.xMax, coor.xMin);
	printf("y: %d %d\n", coor.yMax, coor.yMin);
	printf("z: %d %d\n", coor.zMax, coor.zMin);

	GLint location;

	// modeling transformation matrix
	mat4 T(1.0f);
	glUniformMatrix4fv(1, 1, GL_FALSE, value_ptr(T));

	// modeling transformation matrix
	mat4 M(1.0f);
	M = translate(M, model_state.pos);
	M = rotate(M, 0.0f, vec3(0.0f, 1.0f, 0.0f));
	M = scale(M, model_state.scale);
	M = M * model_state.R;
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

	set_color(0, 0, 0);
	glLineWidth(1.0f);
	glUniform1i(2, 1);
	models[idx_selected]->draw_wire();
	

	// draw the grid
	set_color(1, 1, 1);
	glLineWidth(1.5f);
	glUniform1i(2, 0);
	location = glGetUniformLocation(program, "T");
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(mat4(1.0f)));
	mat4 GM(1.0f);
	GM = translate(GM, vec3(0.0f, -1.0f, 0.0f));
	GM = scale(GM, vec3(0.3f));
	location = glGetUniformLocation(program, "M");
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(GM));
	grid_down.draw();
	grid_up.draw();
	

	glFlush();
}

void keyboard(unsigned char key, int x, int y)
{
	int tmp1, tmp2;
	switch (key)
	{
		// rotate a block about the x-axis by 90 degrees.
	case 'a':
		model_state.R = rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * model_state.R;
		tmp1 = coor.zMax;
		tmp2 = coor.zMin;
		coor.zMax = coor.yMax;
		coor.zMin = coor.yMin;
		coor.yMax = max(tmp1, tmp2);
		coor.yMin = -min(tmp1, tmp2);
		glutPostRedisplay();
		break;

		// rotate a block about the y-axis by 90 degrees.
	case 's':
		model_state.R = rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * model_state.R;
		tmp1 = coor.zMax;
		tmp2 = coor.zMin;
		coor.zMax = -coor.xMax;
		coor.zMin = coor.xMin;
		coor.xMax = tmp1;
		coor.xMin = tmp2;
		glutPostRedisplay();
		break;

		// rotate a block about the z-axis by 90 degrees.
	case 'd':
		model_state.R = rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * model_state.R;
		tmp1 = coor.yMax;
		tmp2 = coor.yMin;
		coor.yMax = coor.xMax;
		coor.yMin = coor.xMin;
		coor.xMax = -tmp1;
		coor.xMin = tmp2;
		glutPostRedisplay();
		break;
	}
}

void specialkey(int key, int x, int y)
{
	using namespace glm;
	mat4 VT = transpose(camera.get_viewing());
	vec3 x_axis(VT[0]);

	x_axis[1] = 0;
	x_axis = normalize(x_axis);
	float angle = (float)(180.0f * acosf(dot(x_axis, vec3(1, 0, 0))) / M_PI);

	if (angle < 45)
	{
		switch (key)
		{
		case GLUT_KEY_LEFT: model_state.pos[0] -= box_cell; break;
		case GLUT_KEY_RIGHT: model_state.pos[0] += box_cell; break;
		case GLUT_KEY_UP: model_state.pos[2] -= box_cell; break;
		case GLUT_KEY_DOWN: model_state.pos[2] += box_cell; break;
		}
	}
	else if (angle < 135)
	{
		if (x_axis[2] > 0)
		{
			switch (key)
			{
			case GLUT_KEY_LEFT: model_state.pos[2] -= box_cell; break;
			case GLUT_KEY_RIGHT: model_state.pos[2] += box_cell; break;
			case GLUT_KEY_UP: model_state.pos[0] += box_cell; break;
			case GLUT_KEY_DOWN: model_state.pos[0] -= box_cell; break;
			}
		}
		else
		{
			switch (key)
			{
			case GLUT_KEY_LEFT: model_state.pos[2] += box_cell; break;
			case GLUT_KEY_RIGHT: model_state.pos[2] -= box_cell; break;
			case GLUT_KEY_UP: model_state.pos[0] -= box_cell; break;
			case GLUT_KEY_DOWN: model_state.pos[0] += box_cell; break;
			}
		}
	}
	else
	{
		switch (key)
		{
		case GLUT_KEY_LEFT: model_state.pos[0] += box_cell; break;
		case GLUT_KEY_RIGHT: model_state.pos[0] -= box_cell; break;
		case GLUT_KEY_UP: model_state.pos[2] += box_cell; break;
		case GLUT_KEY_DOWN: model_state.pos[2] -= box_cell; break;
		}
	}

	glutPostRedisplay();
	// may do any other operations here if needed
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

	if (redraw)
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

void idle()
{
	static clock_t prev_time = clock();
	clock_t curr_time = clock();
	if (1.0 * (curr_time - prev_time) / CLOCKS_PER_SEC > 1.0) {
		// Updated the block to make it go down ... 
		/*if (model_state.pos[1] == -1.0f + (box_cell * 10) + (box_cell / 2)) {
			coor.yMax += 10;
			coor.yMin += 10;
		}*/
		/*if (model_state.pos[1] - box_cell > min_y) {
			model_state.pos[1] -= box_cell;
			coor.yMax--;
			coor.yMin--;
		}*/
			
		glutPostRedisplay();
		prev_time = curr_time;
	}
}

void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
	glutInitWindowSize(512, 512);
	glutCreateWindow("3D Tetris");
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
	glutIdleFunc(idle);
	glutMainLoop();
}