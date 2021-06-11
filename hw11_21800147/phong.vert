#version 430

in vec4 vPosition;
in vec4 vNormal;
out vec4 fNormal;
out vec4 fPosition;

uniform mat4 P;	// projection matrix
uniform mat4 VM;	// VM = V*M

void main(){
	gl_Position = P * VM * vPosition;
	fNormal = vNormal;
	fPosition = vPosition;
}