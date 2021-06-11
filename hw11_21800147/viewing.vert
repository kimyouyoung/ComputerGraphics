#version 430

out vec4 fColorFront;	// color for the front-facing fragment
out vec4 fColorBack;	// color for the back-facing fragment
in vec4 vPosition;	// vertex position in the modeling coordinate system(MS)
in vec4 vNormal;	// normal in the MS

out vec4 fPosition;
out vec4 fNormal;

layout(location=1) uniform int shading_mode;

uniform vec3 LightPos_ec ;	//light source position in camera space(eye coordinate system)
uniform mat4 P;	// projection matrix
uniform mat4 V;	// viewing transformation matrix
uniform mat4 VM;	// VM = V*M
uniform mat4 U;	// transformation matrix for normals(= transpose(inverse(VM))
uniform mat4 T;

uniform vec3 Ka;	// ambient reflectivity
uniform vec3 Kd;	// diffuse reflectivity
uniform vec3 Ks;	// specular reflectivity
uniform float n;	// specular intensity factor
uniform vec3 Il;	// light source intensity
uniform vec3 Ia;	// ambient light intensity
uniform vec3 c;		// coefficients for light source attenuation

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

void main(){

	switch(shading_mode)
	{
		case 0:
			gl_Position = P * VM * vPosition;
			fNormal = vNormal;
			fPosition = vPosition;
			break;

		case 1:
			vec3 vNormal_ec = vec3(normalize(U * vNormal));
			vec3 vPosition_ec = vec3(VM * vPosition);
			gl_Position = P * vec4(vPosition_ec, 1);

			fColorFront = shading(LightPos_ec, vPosition_ec, vNormal_ec);
			fColorBack = shading(LightPos_ec, vPosition_ec, -vNormal_ec);
			break;

		case 2:
			gl_Position = P * VM * T * vPosition;
			break;
			

	}
	
}