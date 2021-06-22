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
};

struct Grid : public Model
{
	Grid(GLfloat w, GLfloat h, int m, int n, int l)
	{
		get_grid(vtx_pos, w, h, m, m, l);

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

struct LshapeModel : public Model
{

	CubePrimitive* cube;

	LshapeModel(
		CubePrimitive* cube
	)
	{
		this->cube = cube;
	}

	glm::mat4 transf(
		GLfloat tx, GLfloat ty, GLfloat tz,
		bool set_uniform = true)
	{
		using namespace glm;
		mat4 T(1.0f);
		T = translate(T, vec3(tx, ty, tz));
		T = scale(T, vec3(1.0f));
		if (set_uniform) {
			glUniformMatrix4fv(1, 1, GL_FALSE, value_ptr(T));
		}

		return T;
	}

	virtual void draw()
	{
		using namespace glm;

		transf(0.0f, 0.0f, 0.0f);
		cube->draw();

		transf(1.0f, 0.0f, 0.0f);
		cube->draw();

		transf(0.0f, 1.0f, 0.0f);
		cube->draw();

		transf(0.0f, 1.0f * 2, 0.0f);
		cube->draw();

	}
};

struct IshapeModel : public Model
{
	CubePrimitive* cube;

	IshapeModel(
		CubePrimitive* cube
	)
	{
		this->cube = cube;
	}

	glm::mat4 transf(
		GLfloat tx, GLfloat ty, GLfloat tz,
		bool set_uniform = true)
	{
		using namespace glm;
		mat4 T(1.0f);
		T = translate(T, vec3(tx, ty, tz));
		T = scale(T, vec3(1.0f));
		if (set_uniform) {
			glUniformMatrix4fv(1, 1, GL_FALSE, value_ptr(T));
		}

		return T;
	}

	virtual void draw()
	{
		using namespace glm;

		transf(0.0f, 0.0f, 0.0f);
		cube->draw();

		transf(0.0f, 1.0f , 0.0f);
		cube->draw();

		transf(0.0f, 1.0f * 2, 0.0f);
		cube->draw();

		transf(0.0f, 1.0f * 3, 0.0f);
		cube->draw();

	}
};

struct BoxshapeModel : public Model
{
	CubePrimitive* cube;

	BoxshapeModel(
		CubePrimitive* cube
	)
	{
		this->cube = cube;
	}

	glm::mat4 transf(
		GLfloat tx, GLfloat ty, GLfloat tz,
		bool set_uniform = true)
	{
		using namespace glm;
		mat4 T(1.0f);
		T = translate(T, vec3(tx, ty, tz));
		T = scale(T, vec3(1.0f));
		if (set_uniform) {
			glUniformMatrix4fv(1, 1, GL_FALSE, value_ptr(T));
		}

		return T;
	}

	virtual void draw()
	{
		using namespace glm;

		transf(0.0f, 0.0f, 0.0f);
		cube->draw();

		transf(0.0f, 1.0f, 0.0f);
		cube->draw();

		transf(1.0f, 0.0f, 0.0f);
		cube->draw();

		transf(1.0f, 1.0f, 0.0f);
		cube->draw();

	}
};

struct NshapeModel : public Model
{
	CubePrimitive* cube;

	NshapeModel(
		CubePrimitive* cube
	)
	{
		this->cube = cube;
	}

	glm::mat4 transf(
		GLfloat tx, GLfloat ty, GLfloat tz,
		bool set_uniform = true)
	{
		using namespace glm;
		mat4 T(1.0f);
		T = translate(T, vec3(tx, ty, tz));
		T = scale(T, vec3(1.0f));
		if (set_uniform) {
			glUniformMatrix4fv(1, 1, GL_FALSE, value_ptr(T));
		}

		return T;
	}

	virtual void draw()
	{
		using namespace glm;

		transf(0.0f, 0.0f, 0.0f);
		cube->draw();

		transf(0.0f, 1.0f, 0.0f);
		cube->draw();

		transf(-1.0f, 1.0f, 0.0f);
		cube->draw();

		transf(-1.0f, 1.0f * 2, 0.0f);
		cube->draw();

	}
};