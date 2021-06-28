#define _CRT_SECURE_NO_WARNINGS
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
#include <cmath>


using namespace std;

Coordinate coor[4];
CubePrimitive cube(1.0f, 1.0f, 1.0f);
Grid grid_down(5, 5, 5, 5, 0);
Grid grid_up(5, 5, 5, 5, 10);

LshapeModel LShape(&cube, &(coor[0]));
IshapeModel IShape(&cube, &(coor[1]));
BoxshapeModel BoxShape(&cube, &(coor[2]));
NshapeModel NShape(&cube, &(coor[3]));
vector<Model*> models;

GLuint program;
int idx_selected = 2;
bool show_wireframe = true;

bool stack[5][10][5] = { false };
int xAxis[4] = { 0 };
int yAxis[4] = { 0 };
int zAxis[4] = { 0 };

bool over = false;
int score = 0;

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
glm::mat4 transf(
	GLfloat tx, GLfloat ty, GLfloat tz,
	bool set_uniform = true)
{
	using namespace glm;
	mat4 T(1.0f);
	T = translate(T, vec3(tx, ty, tz));
	T = scale(T, vec3(0.3f));

	return T;
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

	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 10; j++) {
			for (int l = 0; l < 5; l++) {
				if (stack[i][j][l]) {
					location = glGetUniformLocation(program, "T");
					glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(mat4(1.0f)));
					mat4 ST = transf(-0.6f + (i * box_cell), -0.85f + (j * box_cell), 0.6f - (l * box_cell));
					location = glGetUniformLocation(program, "M");
					glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(ST));
					models[0]->draw();
				}
			}
		}
	}

	glDisable(GL_POLYGON_OFFSET_FILL);
	if (show_wireframe)
	{
		set_color(0, 0, 0);
		glLineWidth(1.0f);
		glUniform1i(2, 1);
		for (int i = 0; i < 5; i++) {
			for (int j = 0; j < 10; j++) {
				for (int l = 0; l < 5; l++) {
					if (stack[i][j][l]) {
						location = glGetUniformLocation(program, "T");
						glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(mat4(1.0f)));
						mat4 ST = transf(-0.6f + (i * box_cell), -0.85f + (j * box_cell), 0.6f - (l * box_cell));
						location = glGetUniformLocation(program, "M");
						glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(ST));
						models[0]->draw_wire();
					}
				}
			}
		}
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);
	glUniform1i(2, 0);

	// modeling transformation matrix
	mat4 M(1.0f);
	M = translate(M, model_state.pos);
	M = rotate(M, 0.0f, vec3(0.0f, 1.0f, 0.0f));
	M = scale(M, model_state.scale);
	M = M * model_state.R;

	location = glGetUniformLocation(program, "M");
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(M));


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
	bool pass = true;
	switch (key)
	{
		// rotate a block about the x-axis by 90 degrees.
	case 'a':
		glm::mat4 A = rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		for (int i = 0; i < 4; i++) {
			glm::vec4 point((float)xAxis[i], (float)yAxis[i], (float)zAxis[i], 1);
			glm::vec4 result = A * point;
			yAxis[i] = (int)result[1] + yAxis[i];
			zAxis[i] = (int)(result[2]) % 2 + 1 ;
			if (zAxis[i] < -2 && zAxis[i] > 2)
				pass = false;
			if (yAxis[i] < 0)
				pass = false;
			/*for (int j = 0; j < 5; j++) {
				for (int l = 0; l < 5; l++) {
					if (yAxis[i] < 10 && stack[j][yAxis[i]][l])
						pass = false;
				}
			}*/
		}
		if(pass)
			model_state.R = rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * model_state.R;

		glutPostRedisplay();
		break;

		// rotate a block about the y-axis by 90 degrees.
	case 's':
		glm::mat4 S = rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		for (int i = 0; i < 4; i++) {
			glm::vec4 point((float)xAxis[i], (float)yAxis[i], (float)zAxis[i], 1);
			glm::vec4 result = S * point;
			xAxis[i] = (int)result[0];
			zAxis[i] = (int)result[2];
			if (zAxis[i] < -2 && zAxis[i] > 2)
				pass = false;
			if (xAxis[i] < -2 && xAxis[i] > 2) 
				pass = false;
			for (int j = 0; j < 10; j++) {
				if (xAxis[i] + 2 >= 0 && xAxis[i] + 2 < 5 && abs(zAxis[i] - 2) >= 0 && abs(zAxis[i] - 2) < 5) {
					if (stack[xAxis[i] + 2][j][abs(zAxis[i] - 2)])
						pass = false;
				}
			}
		}
		if (pass)
			model_state.R = rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * model_state.R;
		glutPostRedisplay();
		break;

		// rotate a block about the z-axis by 90 degrees.
	case 'd':
		model_state.R = rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * model_state.R;
		
		glutPostRedisplay();
		break;

	case 'w':
		show_wireframe = !show_wireframe;
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

	int signX = model_state.pos[0] / abs(model_state.pos[0]);
	int signZ = model_state.pos[2] / abs(model_state.pos[2]);

	if (angle < 45)
	{
		switch (key)
		{
		case GLUT_KEY_LEFT:
			if (signX * abs(model_state.pos[0] * coor[idx_selected - 1].xMin) > -0.6f) {
				model_state.pos[0] -= box_cell;
				coor[idx_selected - 1].xMin--;
				coor[idx_selected - 1].xMax--;
				for (int i = 0; i < 4; i++)
					xAxis[i] -=  1;
			} 
			break;
		case GLUT_KEY_RIGHT:
			if (signX * abs(model_state.pos[0] * coor[idx_selected - 1].xMax) < 0.6f) {
				model_state.pos[0] += box_cell;
				coor[idx_selected - 1].xMin++;
				coor[idx_selected - 1].xMax++;
				for (int i = 0; i < 4; i++)
					xAxis[i] += 1;
			}
			break;
		case GLUT_KEY_UP:
			if (signZ * abs(model_state.pos[2] * coor[idx_selected - 1].zMin) > -0.6f) {
				model_state.pos[2] -= box_cell;
				coor[idx_selected - 1].zMin--;
				coor[idx_selected - 1].zMax--;
				for (int i = 0; i < 4; i++)
					zAxis[i] -= 1;
			}
			break;
		case GLUT_KEY_DOWN: 
			if (signZ * abs(model_state.pos[2] * coor[idx_selected - 1].zMax) < 0.6f) {
				model_state.pos[2] += box_cell;
				coor[idx_selected - 1].zMin++;
				coor[idx_selected - 1].zMax++;
				for (int i = 0; i < 4; i++)
					zAxis[i] += 1;
			}
			break;
		}
	}
	else if (angle < 135)
	{
		if (x_axis[2] > 0)
		{
			switch (key)
			{
			case GLUT_KEY_LEFT: 
				if (signZ * abs(model_state.pos[2] * coor[idx_selected - 1].zMin) > -0.6f) {
					model_state.pos[2] -= box_cell;
					coor[idx_selected - 1].zMin--;
					coor[idx_selected - 1].zMax--;
					for (int i = 0; i < 4; i++)
						zAxis[i] -= 1;
				}
				break;
			case GLUT_KEY_RIGHT: 
				if (signZ * abs(model_state.pos[2] * coor[idx_selected - 1].zMax) < 0.6f) {
					model_state.pos[2] += box_cell;
					coor[idx_selected - 1].zMin++;
					coor[idx_selected - 1].zMax++;
					for (int i = 0; i < 4; i++)
						zAxis[i] += 1;
				}
				break;
			case GLUT_KEY_UP: 
				if (signX * abs(model_state.pos[0] * coor[idx_selected - 1].xMax) < 0.6f) {
					model_state.pos[0] += box_cell;
					coor[idx_selected - 1].xMin++;
					coor[idx_selected - 1].xMax++;
					for (int i = 0; i < 4; i++)
						xAxis[i] += 1;
				}
				break;
			case GLUT_KEY_DOWN: 
				if (signX * abs(model_state.pos[0] * coor[idx_selected - 1].xMin) > -0.6f) {
					model_state.pos[0] -= box_cell;
					coor[idx_selected - 1].xMin--;
					coor[idx_selected - 1].xMax--;
					for (int i = 0; i < 4; i++)
						xAxis[i] -= 1;
				}
				break;
			}
		}
		else
		{
			switch (key)
			{
			case GLUT_KEY_LEFT: 
				if (signZ * abs(model_state.pos[2] * coor[idx_selected - 1].zMax) < 0.6f) {
					model_state.pos[2] += box_cell;
					coor[idx_selected - 1].zMin++;
					coor[idx_selected - 1].zMax++;
					for (int i = 0; i < 4; i++)
						zAxis[i] += 1;
				}
				break;
			case GLUT_KEY_RIGHT: 
				if (signZ * abs(model_state.pos[2] * coor[idx_selected - 1].zMin) > -0.6f) {
					model_state.pos[2] -= box_cell;
					coor[idx_selected - 1].zMin--;
					coor[idx_selected - 1].zMax--;
					for (int i = 0; i < 4; i++)
						zAxis[i] -= 1;
				}
				break;
			case GLUT_KEY_UP: 
				if (signX * abs(model_state.pos[0] * coor[idx_selected - 1].xMax) < 0.6f) {
					model_state.pos[0] += box_cell;
					coor[idx_selected - 1].xMin++;
					coor[idx_selected - 1].xMax++;
					for (int i = 0; i < 4; i++)
						xAxis[i] += 1;
				}
				break;
			case GLUT_KEY_DOWN:
				if (signX * abs(model_state.pos[0] * coor[idx_selected - 1].xMin) > -0.6f) {
					model_state.pos[0] -= box_cell;
					coor[idx_selected - 1].xMin--;
					coor[idx_selected - 1].xMax--;
					for (int i = 0; i < 4; i++)
						xAxis[i] -= 1;
				}
				break;
			}
		}
	}
	else
	{
		switch (key)
		{
		case GLUT_KEY_LEFT: 
			if (signX * abs(model_state.pos[0] * coor[idx_selected - 1].xMax) < 0.6f) {
				model_state.pos[0] += box_cell;
				coor[idx_selected - 1].xMin++;
				coor[idx_selected - 1].xMax++;
				for (int i = 0; i < 4; i++)
					xAxis[i] += 1;
			}
			break;
		case GLUT_KEY_RIGHT:
			if (signX * abs(model_state.pos[0] * coor[idx_selected - 1].xMin) > -0.6f) {
				model_state.pos[0] -= box_cell;
				coor[idx_selected - 1].xMin--;
				coor[idx_selected - 1].xMax--;
				for (int i = 0; i < 4; i++)
					xAxis[i] -= 1;
			}
			break;
		case GLUT_KEY_UP: 
			if (signZ * abs(model_state.pos[2] * coor[idx_selected - 1].zMax) < 0.6f) {
				model_state.pos[2] += box_cell;
				coor[idx_selected - 1].zMin++;
				coor[idx_selected - 1].zMax++;
				for (int i = 0; i < 4; i++)
					zAxis[i] += 1;
			}
			break;
		case GLUT_KEY_DOWN:
			if (signZ * abs(model_state.pos[2] * coor[idx_selected - 1].zMin) > -0.6f) {
				model_state.pos[2] -= box_cell;
				coor[idx_selected - 1].zMin--;
				coor[idx_selected - 1].zMax--;
				for (int i = 0; i < 4; i++)
					zAxis[i] -= 1;
			}
			break;
		}
	}

	glutPostRedisplay();
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

void coordinateInit(int idx)
{
	if (idx == 1) {
		coor[idx - 1].xMin = 0;
		coor[idx - 1].xMax = 1;
		coor[idx - 1].yMin = 0;
		coor[idx - 1].yMax = 2;
		coor[idx - 1].zMin = 0;
		coor[idx - 1].zMax = 0;
	}
	else if (idx == 2) {
		coor[idx - 1].xMin = 0;
		coor[idx - 1].xMax = 0;
		coor[idx - 1].yMin = 0;
		coor[idx - 1].yMax = 3;
		coor[idx - 1].zMin = 0;
		coor[idx - 1].zMax = 0;
	}
	else if (idx == 3) {
		coor[idx - 1].xMin = 0;
		coor[idx - 1].xMax = 1;
		coor[idx - 1].yMin = 0;
		coor[idx - 1].yMax = 1;
		coor[idx - 1].zMin = 0;
		coor[idx - 1].zMax = 0;
	}
	else if (idx == 4)
	{
		coor[idx - 1].xMin = -1;
		coor[idx - 1].xMax = 0;
		coor[idx - 1].yMin = 0;
		coor[idx - 1].yMax = 2;
		coor[idx - 1].zMin = 0;
		coor[idx - 1].zMax = 0;
	}
}

void modelStateInit()
{
	coordinateInit(idx_selected);
	idx_selected = rand() % 4 + 1;

	model_state.pos[0] = 0.0f;
	model_state.pos[1] = -1.0f + (box_cell * 10) + (box_cell / 2);
	model_state.pos[2] = 0.0f;

	model_state.R = glm::mat4(1.0f);

	if (idx_selected == 1) {
		xAxis[0] = 0; yAxis[0] = 10; zAxis[0] = 0;
		xAxis[1] = 1; yAxis[1] = 10; zAxis[1] = 0;
		xAxis[2] = 0; yAxis[2] = 11; zAxis[2] = 0;
		xAxis[3] = 0; yAxis[3] = 12; zAxis[3] = 0;
	}
	else if (idx_selected == 2) {
		xAxis[0] = 0; yAxis[0] = 10; zAxis[0] = 0;
		xAxis[1] = 0; yAxis[1] = 11; zAxis[1] = 0;
		xAxis[2] = 0; yAxis[2] = 12; zAxis[2] = 0;
		xAxis[3] = 0; yAxis[3] = 13; zAxis[3] = 0;
	}
	else if (idx_selected == 3) {
		xAxis[0] = 0; yAxis[0] = 10; zAxis[0] = 0;
		xAxis[1] = 1; yAxis[1] = 10; zAxis[1] = 0;
		xAxis[2] = 0; yAxis[2] = 11; zAxis[2] = 0;
		xAxis[3] = 1; yAxis[3] = 11; zAxis[3] = 0;
	}
	else if (idx_selected == 4) {
		xAxis[0] = 0; yAxis[0] = 10; zAxis[0] = 0;
		xAxis[1] = 0; yAxis[1] = 11; zAxis[1] = 0;
		xAxis[2] = -1; yAxis[2] = 11; zAxis[2] = 0;
		xAxis[3] = -1; yAxis[3] = 12; zAxis[3] = 0;
	}
}

void newGame()
{
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 10; j++) {
			for (int l = 0; l < 5; l++) {
				stack[i][j][l] = false;
			}
		}
	}
	modelStateInit();
	score = 0;
}

void clearPlane()
{
	for (int y = 0; y < 10; y++) {
		for (int x = 0; x < 5; x++) {
			for (int z = 0; z < 5; z++) {
				if (y == 9) {
					stack[x][y][z] = false;
				}
				else {
					stack[x][y][z] = stack[x][y + 1][z];
				}
			}
		}
	}
}

void idle()
{
	static clock_t prev_time = clock();
	clock_t curr_time = clock();
	bool check = false;
	char yesOrno;
	if (1.0 * (curr_time - prev_time) / CLOCKS_PER_SEC > 1.0) {

		if (over) {
			printf("Game is over and your game score is %d.\n", score);
			printf("Do you want another game? (y/n) ");
			scanf(" %c", &yesOrno);
			if (yesOrno == 'y') {
				over = false;
				newGame();
			}
			else {
				exit(0);
			}
		}

		for (int i = 0; i < 4; i++) {
			if ((yAxis[i] < 10 && yAxis[i] >= 0) && stack[xAxis[i] + 2][yAxis[i] - 1][abs(zAxis[i]-2)]) {
				check = true;
				break;
			}
		}

		// Updated the block to make it go down ... 
		if (model_state.pos[1] > min_y + box_cell && !check) {
			model_state.pos[1] -= box_cell;
			for (int i = 0; i < 4; i++)
				yAxis[i] -= 1;
		}
		else {
			
			for (int i = 0; i < 4; i++) {
				if (xAxis[i]+2 >= 0 && xAxis[i]+2 < 5 && yAxis[i] >= 0 && yAxis[i] < 10 && abs(zAxis[i]-2) >= 0 && abs(zAxis[i] - 2) < 5)
					stack[xAxis[i]+2][yAxis[i]][abs(zAxis[i]-2)] = true;
			}
			if (!over)
				modelStateInit();

		}

		
		for (int y = 0; y < 10; y++) {
			bool s[5][5] = { false };
			for (int x = 0; x < 5; x++) {
				for (int z = 0; z < 5; z++) {
					if (stack[x][y][z])
						s[x][z] = true;
				}
			}
			bool c = true;
			for (int i = 0; i < 5; i++) {
				for (int j = 0; j < 5; j++) {
					if (!s[i][j]) {
						c = false;
						break;
					}
				}
				if (!c)
					break;
			}
			if (c) {
				score++;
				clearPlane();
			}
			else
				break;
		}

		for (int i = 0; i < 5; i++) {
			for (int j = 0; j < 5; j++) {
				if (stack[i][9][j]) {
					over = true;
					break;
				}
			}
			if (over)
				break;
		}

		if (!over) {
			glutPostRedisplay();
			prev_time = curr_time;
		}
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

	srand(time(NULL));

	idx_selected = rand() % 4 + 1;
	modelStateInit();

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