#include "shader.h"
#include "glerror.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <memory>

ShaderProgram::ShaderProgram() :m_vShader(0), m_fShader(0), m_program(0), m_linked(false)
{
	check_gl_error();
	m_vShader = glCreateShader(GL_VERTEX_SHADER);
	check_gl_error();
	m_fShader = glCreateShader(GL_FRAGMENT_SHADER);
	check_gl_error();
	m_program = glCreateProgram();
	check_gl_error();
}

ShaderProgram::ShaderProgram(ShaderProgram&& rhs) :m_vShader(rhs.m_vShader), m_fShader(rhs.m_fShader), m_program(rhs.m_program), m_linked(rhs.m_linked)
{
	rhs.m_vShader = 0;
	rhs.m_fShader = 0;
	rhs.m_program = 0;
	rhs.m_linked = false;
}

ShaderProgram::~ShaderProgram()
{
	if (s_boundProgram == m_program)
	{
		glUseProgram(0);
		check_gl_error();
		s_boundProgram = 0;
	}
	if (m_program)
	{ 
		glDeleteProgram(m_program);
		check_gl_error();
	}
	if (m_vShader)
	{
		glDeleteShader(m_vShader);
		check_gl_error();
	}
	if (m_fShader)
	{
		glDeleteShader(m_fShader);
		check_gl_error();
	}
}

bool ShaderProgram::loadFromFile(std::string const& vertexShader, std::string const& fragmentShader)
{
	std::ifstream vertexFile(vertexShader, std::ios::in);
	std::ifstream fragmentFile(fragmentShader, std::ios::in);
	bool shadersCompiled = true;

	std::string contentString{ std::istreambuf_iterator<char>(vertexFile), std::istreambuf_iterator<char>() };
	const char* fileContent = contentString.c_str();
	int fileSize = contentString.size();
	glShaderSource(m_vShader, 1, &fileContent, &fileSize);
	check_gl_error();
	glCompileShader(m_vShader);
	check_gl_error();
	GLint params;
	glGetShaderiv(m_vShader, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params)
	{
		glGetShaderiv(m_vShader, GL_INFO_LOG_LENGTH, &params);
		char* infoLog = new char[params];
		glGetShaderInfoLog(m_vShader, params, &params, infoLog);
		std::cerr << "Cannot compile vertex shader, log: " << infoLog << std::endl;
		delete[] infoLog;
		shadersCompiled = false;
	}
	check_gl_error();

	contentString.assign( std::istreambuf_iterator<char>(fragmentFile), std::istreambuf_iterator<char>() );
	fileContent = contentString.c_str();
	fileSize = contentString.size();
	glShaderSource(m_fShader, 1, &(fileContent), &fileSize);
	check_gl_error();
	glCompileShader(m_fShader);
	check_gl_error();
	glGetShaderiv(m_fShader, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params)
	{
		glGetShaderiv(m_fShader, GL_INFO_LOG_LENGTH, &params);
		char* infoLog = new char[params];
		glGetShaderInfoLog(m_fShader, params, &params, infoLog);
		std::cerr << "Cannot compile fragment shader, log: " << infoLog << std::endl;
		delete[] infoLog;
		shadersCompiled = false;
	}
	check_gl_error();

	if (shadersCompiled)
	{
		glAttachShader(m_program, m_vShader);
		check_gl_error();
		glAttachShader(m_program, m_fShader);
		check_gl_error();
		glLinkProgram(m_program);
		check_gl_error();
		glGetProgramiv(m_program, GL_LINK_STATUS, &params);
		if (GL_TRUE != params)
		{
			glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &params);
			char* infoLog = new char[params];
			glGetProgramInfoLog(m_program, params, &params, infoLog);
			std::cerr << "Linking failure, log: " << infoLog << std::endl;
			delete[] infoLog;
		}
		else
		{
			m_linked = true;
			return true;
		}
	}
	check_gl_error();

	return false;
}

GLint ShaderProgram::uniformLocation(const std::string& name)
{
	auto maploc = m_uniformLocs.find(name);
	if (maploc != m_uniformLocs.cend())
		return maploc->second;
	check_gl_error();
	GLint loc = glGetUniformLocation(m_program, name.c_str());
	check_gl_error();
	m_uniformLocs.insert(maploc, std::make_pair(name, loc));
	return loc;
}

void ShaderProgram::use()
{
	if (!m_linked)
	{
		std::cerr << "Can not use program that was not linked" << std::endl;
		return;
	}
	if (s_boundProgram != m_program)
	{
		check_gl_error();
		glUseProgram(m_program);
		s_boundProgram = m_program;
		check_gl_error();
	}
}

GLuint ShaderProgram::s_boundProgram = 0;
