#pragma once

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define _USE_MATH_DEFINES
#include <math.h>

struct Camera
{
	enum { ORTHOGRAPHIC, PERSPECTIVE };
	glm::vec3 eye;
	glm::vec3 center;
	glm::vec3 up;
	float zoom_factor;
	int c_projection_mode;
	float z_near;
	float z_far;
	float fovy;
	float x_right;

	Camera() :
		eye(0, 0, 8),
		center(0, 0, 0),
		up(0, 1, 0),
		zoom_factor(1.0f),
		c_projection_mode(PERSPECTIVE),
		z_near(0.01f),
		z_far(100.0f),
		fovy((float)(M_PI / 180.0 * (30.0))),
		x_right(1.2f)
	{}

	glm::mat4 get_viewing() { return glm::lookAt(eye, center, up); }

	glm::mat4 get_projection(float aspect)
	{
		glm::mat4 P(1.0f);
		switch (c_projection_mode){
		case ORTHOGRAPHIC:
			P = parallel((double)zoom_factor * x_right, aspect, z_near, z_far);
			break;

		case PERSPECTIVE:
			P = glm::perspective(zoom_factor * fovy, aspect, z_near, z_far);
			break;
		}
		return P;
	}

	glm::mat4 parallel(double r, double aspect, double n, double f)
	{
		double l = -r;
		double width = 2 * r;
		double height = width / aspect;
		double t = height / 2;
		double b = -t;

		return glm::ortho(l, r, b, t, n, f);
	}
};