// GLSL version to use
#version 430

// Declaration of vertex shader's input attribute
in vec4 vPosition;

void main()
{
   // Built-in 4D vector representing the final processed vertex position(gl_Position)
   gl_Position = vPosition;
}
