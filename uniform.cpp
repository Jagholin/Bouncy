#include <GL/glew.h>
#include "uniform.h"
#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

Uniform::Uniform(const std::string& name):
m_name(name),
m_noUniform(false)
{
	// no op
}

Uniform::~Uniform()
{

}

bool Uniform::overrides(const Uniform& otherUniform)
{
	return otherUniform.m_name == m_name;
}

std::string Uniform::name() const
{
	return m_name;
}

GLint Uniform::pre_apply_impl(ShaderProgram* prog)
{
	GLint loc = prog->uniformLocation(m_name);
	if (loc == -1)
	{
		if (!m_noUniform)
		{
			m_noUniform = true;
			std::cerr << "Shader doesn't have a uniform named " << m_name << "\n";
		}
	}
	return loc;
}

void Uniform::apply_impl(ShaderProgram* prog, glm::vec2 value)
{
	GLint loc = pre_apply_impl(prog);
	glUniform2fv(loc, 1, glm::value_ptr(value));
}

void Uniform::apply_impl(ShaderProgram* prog, glm::vec3 value)
{
	GLint loc = pre_apply_impl(prog);
	glUniform3fv(loc, 1, glm::value_ptr(value));
}

void Uniform::apply_impl(ShaderProgram *prog, float value)
{
	GLint loc = pre_apply_impl(prog);
	glUniform1f(loc, value);
}

void Uniform::apply_impl(ShaderProgram* prog, int value)
{
	GLint loc = pre_apply_impl(prog);
	glUniform1i(loc, value);
}

void Uniform::apply_impl(ShaderProgram* prog, glm::mat4 value)
{
	GLint loc = pre_apply_impl(prog);
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}
