#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "LoadShaders.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "camera.h"
#include "primi.h"
#include "loadobj.h"

using namespace std;
typedef std::vector<GLfloat> GLvec;

#define UVAR(name, i)	glGetUniformLocation(program, name)
#define UVARS(name)		UVAR(name, shading_mode)

int shading_mode = 0;

Grid grid(5, 5, 10, 10);

// for model state -> translate, scale, rotate
struct ModelState {
	glm::vec3 pos;
	glm::vec3 scale;
	GLfloat theta;

	ModelState() : pos(0), scale(0.7), theta(0) {}
} model_state;

float light = 3.0f;

std::vector<tinyobj::real_t> vertices;
std::vector<tinyobj::real_t> normals;
std::vector<tinyobj::real_t> colors;
std::vector<std::vector<size_t>> vertex_map;
std::vector<std::vector<size_t>> material_map;
std::vector<tinyobj::shape_t> shapes;
std::vector<tinyobj::material_t> materials;
bool is_obj_valid = false;

const char* obj_filepath[2] = {
	"models/bunny.obj",
	"models/sportsCar.obj",
};

GLuint program;
GLuint vao;
GLuint vbo[2];

int model = 0;

bool show_wireframe = false;
bool show_grid = true;

int button_pressed[3] = { GLUT_UP, GLUT_UP, GLUT_UP };
int mouse_pos[2] = { 0, 0 };

Camera camera;

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

void bind_buffer(GLint buffer, GLvec& vec, int program, const GLchar* attri_name, GLint attri_size)
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

void bind_buffer(GLint buffer, int program, const GLchar* attri_name, GLint attri_size)
{
	// bind the buffer object to a specific type.
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	// returns the index of a general vertex attribute bound to that attribute variable.
	GLuint location = glGetAttribLocation(program, attri_name);
	// the specified vertex shader attribute variable and the data information stored in the vertex buffer object are stored in the VAO.
	glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0);
	// active location
	glEnableVertexAttribArray(location);
}
void init()
{
	// add menu(mouse right button)
	int menu_id = glutCreateMenu(cb_main_menu);
	glutAddMenuEntry("Orthographic projection", 0);
	glutAddMenuEntry("Perspective projection", 1);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	ShaderInfo shaders[] = {
		{GL_VERTEX_SHADER, "viewing.vert"},
		{GL_FRAGMENT_SHADER, "viewing.frag"},
		{GL_NONE, NULL}
	};
	
	program = LoadShaders(shaders);
	glUseProgram(program);

	tinyobj::attrib_t attrib;
	is_obj_valid = load_obj(obj_filepath[model], "models/",
		vertices, normals, vertex_map, material_map, attrib, shapes, materials);

	colors.resize(vertices.size());

	// grid initialized
	grid.init(program);

	// Generate vertex array objects
	glGenVertexArrays(1, &vao);
	glGenBuffers(2, vbo);

	// Bind vertex buffer objects
	glBindVertexArray(vao);
	bind_buffer(vbo[0], vertices, program, "vPosition", 3);
	bind_buffer(vbo[1], normals, program, "vNormal", 3);
	
	// active depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
}

void draw_obj_model(int color_mode)
{
	glBindVertexArray(vao);
	glUniform1i(UVARS("ColorMode"), color_mode);

	using namespace glm;
	using namespace tinyobj;

	for (size_t i = 0; i < shapes.size(); ++i) {
		for (size_t j = 0; j < material_map[i].size(); ++j) {
			// set material properties
			int m_id = material_map[i][j];
			if (m_id < 0) {
				glUniform1f(UVARS("n"), 10.0f);
				glUniform3f(UVARS("Ka"), 0.3f, 0.3f, 0.3f);
				glUniform3f(UVARS("Kd"), 1.0f, 1.0f, 1.0f);
				glUniform3f(UVARS("Ks"), 0.8f, 0.8f, 0.8f);
			}
			else {
				glUniform1f(UVARS("n"), materials[m_id].shininess);
				glUniform3fv(UVARS("Ka"), 1, materials[m_id].ambient);
				glUniform3fv(UVARS("Kd"), 1, materials[m_id].diffuse);
				glUniform3fv(UVARS("Ks"), 1, materials[m_id].specular);
			}
			glDrawArrays(GL_TRIANGLES, vertex_map[i][j], vertex_map[i][j + 1] - vertex_map[i][j]);
		}
	}
}

void display()
{
	using namespace glm;
	using namespace tinyobj;

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);

	glUniform1i(1, shading_mode);

	GLint location;

	// modeling transformation matrix
	mat4 M(1.0f);
	// apply model state -> translate: pos, rotate: theta, scale: scale)
	M = translate(M, model_state.pos);
	M = rotate(M, model_state.theta, vec3(0.0f, 1.0f, 0.0f));
	M = scale(M, model_state.scale);

	// viewing transformation matrix
	mat4 V = camera.get_viewing();
	// apply to "V": viewing
	location = glGetUniformLocation(program, "V");
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(V));

	mat4 VM = V * M;
	location = glGetUniformLocation(program, "VM");
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(VM));

	location = glGetUniformLocation(program, "U");
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(transpose(inverse(VM))));

	vec4 LightPos_wc = vec4(10, 10, 3, 1);
	vec3 LightPos_ec = vec3(V * LightPos_wc);
	location = glGetUniformLocation(program, "LightPos_ec");
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(LightPos_ec));

	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);
	double aspect = 1.0 * width / height;

	// projection transformation
	mat4 P = camera.get_projection(aspect);
	// apply to "P": projection
	location = glGetUniformLocation(program, "P");
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(P));

	// light source intensity
	vec3 Il = vec3(light, light, light);
	glUniform3fv(UVARS("Il"), 1, value_ptr(Il));

	// ambient light intensity
	vec3 Ia = vec3(0.8, 0.8, 0.8);
	glUniform3fv(UVARS("Ia"), 1, value_ptr(Ia));

	// coefficients for light source attenuation
	vec3 c = vec3(0.00001, 0.001, 0.0000001);
	glUniform3fv(UVARS("c"), 1, value_ptr(c));

	glDisable(GL_POLYGON_OFFSET_FILL);

	if (is_obj_valid) {
		// Draw the imported model
		draw_obj_model(0);
	}

	if (show_wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		draw_obj_model(1);
	}

	// draw the grid
	if (show_grid)
	{
		// set color: white
		glUniform1i(1, 2);
		glLineWidth(1.0f);
		location = glGetUniformLocation(program, "T");
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

	case 'g':
		show_grid = !show_grid;
		glutPostRedisplay();
		break;

	case 'e':
		glutLeaveMainLoop();
		break;

	// phong shading
	case '1':
	// gouraud shading
	case '2':
		shading_mode = key - '1';
		printf("shading mode: %d\n", shading_mode);
		glutPostRedisplay();
		break;
	}
}

void specialkey(int key, int x, int y)
{
	switch (key)
	{
	// add light intensity 
	case GLUT_KEY_UP:
		light += 0.1f;
		glutPostRedisplay();
		break;

	// substract light intensity 
	case GLUT_KEY_DOWN:
		light -= 0.1f;
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

	if (dir > 0)
		camera.zoom_factor *= 0.95;
	else
		camera.zoom_factor *= 1.05f;
	
	glutPostRedisplay();
}

void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
	glutInitWindowSize(512, 512);
	glutCreateWindow("Obj Viewer");
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	printf("************************************************\n");
	printf("\tKey_UP: light intensity +\n\tKey_Down: light intensity -\n");
	printf("************************************************\n\n");

	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialkey);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutMouseWheelFunc(mouse_wheel);

	// 0: bunny object, 1: sportCar model
	printf("Select number 0(bunny) or 1(sportsCar): ");
	scanf_s("%d", &model);
	init();
	glutDisplayFunc(display);
	
	glutMainLoop();
}