#pragma once
#include <GL/glew.h>
#include <string>
#include <unordered_map>
#include "ref_ptr.h"

class ShaderProgram : public InnerRefcounter
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

	std::unordered_map<std::string, GLint> m_uniformLocs;
};
