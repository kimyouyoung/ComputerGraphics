#include<stdio.h>
#include<GL/glew.h>
#include<GL/glut.h>
// Built-in header file which defines utility functions for loading shaders
#include"LoadShaders.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <time.h>

// number of vertices
const GLsizei NumVertices = 6;

// Coordinates for each vertex
GLfloat vertices[NumVertices][2] = {
   {-0.90f, -0.90f}, {0.85f, -0.90f},
   {-0.90f, 0.85f}, {0.90f, -0.85f},
   {0.90f, 0.90f}, {-0.85f, 0.90f} };

GLuint Buffers[1], VertexArrays[1];
GLuint program;

void init();
void display();

void main(int argc, char** argv)
{
    // Initialize GLUT to make a window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA);
    glutInitWindowSize(512, 512);
    glutCreateWindow(argv[0]);
    GLenum err = glewInit();

    // Initialize GLEW to load OpenGL extensions
    if (err != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    init();
    // Register a display callback function (When rendering is required in openGL)
    glutDisplayFunc(display);
    // enter the GLUT event processing loop.
    glutMainLoop();
}

// Initializes OpenGL for drawing triangles.
void init()
{
    // Create a new vertex array object.
    glGenVertexArrays(1, VertexArrays);
    // Specify the current active vertex array object.
    glBindVertexArray(VertexArrays[0]);

    // Generate buffer
    glGenBuffers(1, Buffers);
    // Specify the type of data to store in the buffer object and prepare to allocate optimal memory to the buffer object.
    glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
    // It allocates memory space for buffer object and copies vertex data to buffer objects.
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Compile shaders and produce a program to which the complied shaders are attached. Then, register the program in OpenGL.
    ShaderInfo shaders[] = {
       {GL_VERTEX_SHADER, "triangles.vert"},
       {GL_FRAGMENT_SHADER, "triangles.frag"},
       {GL_NONE, NULL}
    };

    program = LoadShaders(shaders);
    glUseProgram(program);

    // Finds the location of a specified vertex attribute.
    GLint location = glGetAttribLocation(program, "vPosition");

    // The specified vertex shader attribute variable and the data information stored in the vertex buffer object are stored in the VAO.
    glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, 0, 0);
    // active location
    glEnableVertexAttribArray(location);
}

// Draws triangles with OpenGL.
void display()
{
    // Clear the buffers for color writing.
    glClear(GL_COLOR_BUFFER_BIT);
    // Specifies the current active VAO.
    glBindVertexArray(VertexArrays[0]);

    using namespace glm;
    GLfloat theta = 0.001f * clock();
    mat4 T(1.0f);
    T = rotate(T, theta, vec3(-1.0f, 1.0f, 0.0f));
    T = scale(T, vec3(0.5f));
    GLuint location = glGetUniformLocation(program, "T");
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(T));

    // Draw triangles
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    // Forces the execution of OpenGL commands in finite time.
    glFlush();

    glutPostRedisplay();
}
