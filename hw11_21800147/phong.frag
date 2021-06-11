#version 430

out vec4 FragColor;
in vec4 fPosition;
in vec4 fNormal;

uniform int ColorMode;	// color mode

uniform vec3 LightPos_ec ;	//light source position in camera space(eye coordinate system)
uniform mat4 P;	// projection matrix
uniform mat4 V;	// viewing transformation matrix
uniform mat4 VM;	// VM = V*M
uniform mat4 U;	// transformation matrix for normals(= transpose(inverse(VM))

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

void main()
{	
	// code for rendering a wireframe
	if(ColorMode == 1){
		FragColor = vec4(0, 0, 0, 1);
		return;
	}
	
	vec3 vNormal_ec = vec3(normalize(U * fNormal));
	vec3 fPosition_ec = vec3(VM * fPosition);
	
	if(gl_FrontFacing)
		FragColor =  shading(LightPos_ec, fPosition_ec, vNormal_ec);
	else
		FragColor = shading(LightPos_ec, fPosition_ec, -vNormal_ec);
}