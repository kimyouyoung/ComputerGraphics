#version 430

in vec4 vPosition;
// determine shading color --> normal vectors
in vec4 vNormal;
in vec4 vColor;

out vec4 fPosition;
out vec4 fNormal;
out vec4 fColor;

layout(location=1) uniform mat4 T;
layout(location=2) uniform int mode;
layout(location=3) uniform vec4 uColor;

uniform mat4 M;
uniform mat4 P;
uniform mat4 V;

// ambient intensity
vec3 Ia = vec3(0.3, 1.0, 0.3);
// incident light intensity
vec3 Il = vec3(1.0, 1.0, 1.0);
// ambient coefficient
float Ka = 0.3;
// specular coefficient
float Ks = 0.5;
// diffusion coefficient
float Kd = 0.8;
float c[3] = {0.01, 0.001, 0.0};
// n: shininess
float n = 10.0;
vec4 LightPos_wc = vec4(10, 10, 3, 1);

vec4 shading(vec3 LightPos_ec, vec3 vPosition_ec, vec3 vNormal_ec)
{
	// normalized normal vectors
	vec3 N = normalize(vNormal_ec);
	// sample light position and sample position
	vec3 L = LightPos_ec - vPosition_ec;
	// make L: unit vector
	float d = length(L); L = L/d;
	// V: view vector -> sample point to the camera(eye)(vec3(0.0): origin = camera position) 
	vec3 V = normalize(vec3(0.0) - vPosition_ec);
	vec3 R = reflect(-L, N);

	// attenuation factor --> depending on the distance : on object
	float fatt = min(1.0 / (c[0] + c[1]*d + c[2]*d*d), 1.0);

	// theta(normal vector and light vector) > 90 --> cos_theta < 0
	float cos_theta = max(dot(N,L),0);
	// alpha(reflection vetor and view vector) > 90 --> cos_alpha < 0
	float cos_alpha = max(dot(V,R),0);

	// form of reflection model
	vec3 I = Ia * Ka + fatt * Il * (Kd * cos_theta + Ks * pow(cos_alpha, n));

	return vec4(I, 1);
}

void main()
{
	switch(mode)
	{
	case 0:
		gl_Position = P * V * M * T * vPosition;
		fColor = vColor;
		break;

	case 1:
		gl_Position = P * V * M * T * vPosition;
		fNormal = vNormal;
		fPosition = vPosition;
		break;

	// gouraud shading mode --> fragment shader
	case 2:
		mat4 VM = V*M*T;
		// n' = ((M(-1))T) * n : transpose normal vector
		mat4 U = transpose(inverse(VM));
		vec3 vNormal_ec = vec3(normalize(U*vNormal));
		vec3 vPosition_ec = vec3(VM * vPosition);
		vec3 LightPos_ec = vec3(V * LightPos_wc);

		gl_Position = P * vec4(vPosition_ec, 1);

		fColor = shading(LightPos_ec, vPosition_ec, vNormal_ec);
		break;
	
	case 3:
		gl_Position = P * V * M * T * vPosition;
		fColor = uColor;
		break;


	}
}