#pragma once

#include "util.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Model {
public:

	GLuint vao, buffs[2];
	GLvec vtx_pos;
	GLvec vtx_clrs;

	virtual void init(GLint program)
	{
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		GLvec* vtx_list[2] = { &vtx_pos, &vtx_clrs };

		glGenBuffers(2, buffs);
		bind_buffer(buffs[0], *vtx_list[0], program, "vPosition", 3);
		bind_buffer(buffs[1], *vtx_list[1], program, "vColor", 3);
	}

	virtual void draw() = 0;
	virtual void draw_wire() {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		draw();
	}
	virtual void draw_points() {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		draw();
	}

};

struct Grid : public Model
{
	Grid(GLfloat w, GLfloat h, int m, int n)
	{
		get_grid(vtx_pos, w, h, m, m);

		int N = (int)vtx_pos.size();
		vtx_clrs.resize(N);
		for (int i = 0; i < N; ++i) {
			vtx_clrs[i] = 1.0f;
		}
	}

	virtual void draw()
	{
		glBindVertexArray(vao);
		glDrawArrays(GL_LINES, 0, vtx_pos.size() / 3);
	}
};

struct CubePrimitive : public Model
{
	CubePrimitive(GLfloat lx, GLfloat ly, GLfloat lz)
	{
		get_cube_3d(vtx_pos, lx, ly, lz);
		get_color_3d_by_pos(vtx_clrs, vtx_pos);
	}

	virtual void draw()
	{
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, vtx_pos.size() / 3);
	}
};

struct SpherePrimitive : public Model
{
	SpherePrimitive(GLfloat radius, GLint lats, GLint longs)
	{
		get_sphere_3d(vtx_pos, radius, lats, longs);
		get_color_3d_by_pos(vtx_clrs, vtx_pos);
	}

	virtual void draw()
	{
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, vtx_pos.size() / 3);
	}
};

struct ConePrimitive : public Model
{
	// 0: index side, 1: index bottom
	std::vector<size_t> idx_list[2];

	ConePrimitive(GLfloat radius, GLfloat height, GLint subdivs)
	{
		get_cone_3d(vtx_pos, idx_list[0], idx_list[1], radius, height, subdivs);
		get_color_3d_by_pos(vtx_clrs, vtx_pos);
	}

	virtual void draw()
	{
		glBindVertexArray(vao);
		for(int i = 0; i < 2; ++i)
			glDrawElements(GL_TRIANGLE_FAN, idx_list[i].size(), GL_UNSIGNED_INT, idx_list[i].data());
	}
};

struct CylinderPrimitive : public Model
{
	// 0: index top, 1: index side, 2: index bottom
	std::vector<size_t> idx_list[3];
	GLuint drawing_mode[3] = { GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLE_FAN };

	CylinderPrimitive(GLfloat radius, GLfloat height, GLint subdivs)
	{
		get_cylinder_3d(vtx_pos, idx_list[0], idx_list[1], idx_list[2], radius, height, subdivs);
		get_color_3d_by_pos(vtx_clrs, vtx_pos);
	}

	virtual void draw()
	{
		glBindVertexArray(vao);
		for (int i = 0; i < 3; ++i) {
			glDrawElements(drawing_mode[i], idx_list[i].size(), GL_UNSIGNED_INT, idx_list[i].data());
		}
	}
};

struct TorusPrimitive : public Model
{
	std::vector<std::vector<size_t>> idx_lists;

	TorusPrimitive(GLfloat radius0, GLfloat radius1, GLint longs, GLint lats)
	{
		get_torus_3d(vtx_pos, idx_lists, radius0, radius1, longs, lats);
		get_color_3d_by_pos(vtx_clrs, vtx_pos);
	}

	virtual void draw()
	{
		glBindVertexArray(vao);
		for (auto iter = idx_lists.begin(); iter != idx_lists.end(); ++iter) {
			glDrawElements(GL_TRIANGLE_STRIP, (*iter).size(), GL_UNSIGNED_INT, (*iter).data());
		}
	}
};

struct CarModel : public Model
{
	CubePrimitive* cube;
	SpherePrimitive* sphere;
	ConePrimitive* cone;
	CylinderPrimitive* cylinder;
	TorusPrimitive* torus;

	GLfloat wheel_angle_L;
	GLfloat wheel_angle_R;

	CarModel(
		CubePrimitive* cube,
		SpherePrimitive* sphere,
		ConePrimitive* cone,
		CylinderPrimitive* cylinder,
		TorusPrimitive* torus)
	{
		this->cube = cube;
		this->sphere = sphere;
		this->cone = cone;
		this->cylinder = cylinder;
		this->torus = torus;

		wheel_angle_L = 0;
		wheel_angle_R = 0;
	}

	virtual void init(GLint program)
	{
		// do nothing.
	}

	void move_forward()
	{
		wheel_angle_L += 0.02f;
		wheel_angle_R += 0.02f;
	}

	void move_backward()
	{
		wheel_angle_L -= 0.02f;
		wheel_angle_R -= 0.02f;
	}

	void turn_left()
	{
		wheel_angle_L -= 0.015f;
		wheel_angle_R += 0.015f;
	}

	void turn_right()
	{
		wheel_angle_L += 0.015f;
		wheel_angle_R -= 0.015f;
	}

	glm::mat4 transf(
		GLfloat sx, GLfloat sy, GLfloat sz,
		GLfloat tx, GLfloat ty, GLfloat tz,
		glm::mat4* T_pre = NULL,
		glm::mat4* T_post = NULL,
		bool set_uniform = true)
	{
		using namespace glm;
		mat4 T(1.0f);
		T = translate(T, vec3(tx, ty, tz));
		T = scale(T, vec3(sx, sy, sz));
		if(T_pre) T = (*T_pre) * T;
		if (T_post) T = T * (*T_post);
		if (set_uniform) {
			glUniformMatrix4fv(1, 1, GL_FALSE, value_ptr(T));
		}

		return T;
	}

	virtual void draw()
	{
		using namespace glm;

		GLfloat theta = 0;
		mat4 Rz = rotate(mat4(1.0f), -2 * theta, vec3(0.0f, 0.0f, 1.0f));
		mat4 Ry = rotate(mat4(1.0f), -theta, vec3(0.0f, 1.0f, 0.0f));
		mat4 R_tire = rotate(mat4(1.0f), radians(90.0f), vec3(1, 0, 0));
		mat4 RL = rotate(R_tire, wheel_angle_L, vec3(0.0f, 1.0f, 0.0f));
		mat4 RR = rotate(R_tire, wheel_angle_R, vec3(0.0f, 1.0f, 0.0f));

		// car main body
		transf(1.2f, 0.4f, 0.6f, +0.0f, -0.2f, 0.0f, &Ry);
		cube->draw();

		// car upper body
		transf(0.6f, 0.6f, 0.6f, -0.3f, +0.3f, 0.0f, &Ry);
		cube->draw();

		// car front body
		mat4 R_fb = rotate(mat4(1.0f), radians(90.0f), vec3(0, 1, 0));
		transf(0.5f, 0.5f, 0.5f, +0.25f, 0.0f, 0.0f, &Ry, &(R_fb * RL));
		cylinder->draw();

		// car roof
		mat4 R_dc = rotate(mat4(1.0f), radians(90.0f), vec3(0, 1, 0));
		transf(1.0f, 0.2f, 1.0f, -0.3f, +0.7f, 0.0f, &Ry);
		cone->draw();

		// car front-right light
		transf(0.1f, 0.1f, 0.1f, +0.6f, -0.2f, -0.2f, &Ry);
		sphere->draw();

		// car front-left light
		transf(0.1f, 0.1f, 0.1f, +0.6f, -0.2f, +0.2f, &Ry);
		sphere->draw();

		// front right tire
		transf(0.3f, 0.3f, 0.3f, +0.3f, -0.4f, -0.4f, &Ry, &RR);
		torus->draw();

		// front left tire
		transf(0.3f, 0.3f, 0.3f, +0.3f, -0.4f, +0.4f, &Ry, &RL);
		torus->draw();

		// rear right tire
		transf(0.3f, 0.3f, 0.3f, -0.3f, -0.4f, -0.4f, &Ry, &RR);
		torus->draw();

		// rear left tire
		transf(0.3f, 0.3f, 0.3f, -0.3f, -0.4f, +0.4f, &Ry, &RL);
		torus->draw();


		// front shaft
		mat4 R_shaft = Rz * rotate(mat4(1.0f), radians(90.0f), vec3(1, 0, 0));
		transf(0.12f, 0.12f, 0.9f, +0.3f, -0.4f, +0.0f, &Ry, &R_shaft);
		cylinder->draw();

		// rear shaft
		transf(0.12f, 0.12f, 0.9f, -0.3f, -0.4f, +0.0f, &Ry, &R_shaft);
		cylinder->draw();

	}
};