#pragma once
#include <GL/glew.h>
#include <string>

class ShaderProgram
{
public:
	ShaderProgram();
	ShaderProgram(const ShaderProgram&) = delete;
	ShaderProgram(ShaderProgram&&);
	~ShaderProgram();

	bool loadFromFile(std::string const& vertexShader, std::string const& fragmentShader);

	GLint uniformLocation(const std::string& name);

	void use();

protected:
	GLuint m_vShader, m_fShader, m_program;
	bool m_linked;
	static GLuint s_boundProgram;
};
