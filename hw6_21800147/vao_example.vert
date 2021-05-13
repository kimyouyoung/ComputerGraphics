#version 430

in vec4 vPosition;
in vec4 vColor;
out vec4 fColor;
layout(location=1) uniform mat4 T;

void main()
{
	gl_Position = T * vPosition;
	fColor = vColor;
}
