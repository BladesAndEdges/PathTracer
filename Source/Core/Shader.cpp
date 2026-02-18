#include "Shader.h"
#include <fstream>
#include <assert.h>

// --------------------------------------------------------------------------------
Shader::Shader(const char* const vertexShaderPath, const char* const fragmentShaderPath)
{
	std::string vertexShaderSource = readSourceCode(vertexShaderPath);
	std::string fragmentShaderSource = readSourceCode(fragmentShaderPath);

	GLuint vertexShader = compile(vertexShaderSource.c_str(), GL_VERTEX_SHADER);
	GLuint fragmentShader = compile(fragmentShaderSource.c_str(), GL_FRAGMENT_SHADER);

	link(vertexShader, fragmentShader);
}

// --------------------------------------------------------------------------------
std::string Shader::readSourceCode(const char* file) const
{
	std::string contents;

	std::ifstream ifs(file);
	bool isReady = ifs.is_open();
	assert(isReady);

	if (isReady)
	{
		contents.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
	}

	return contents;
}

// --------------------------------------------------------------------------------
GLuint Shader::compile(const char* const sourceCode, GLenum shaderType) const
{
	GLuint id;
	GLint success = GL_FALSE;
	char debugLog[512];

	id = glCreateShader(shaderType);
	glShaderSource(id, 1, &sourceCode, nullptr);
	glCompileShader(id);

	glGetShaderiv(id, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(id, 512, nullptr, debugLog);
		throw std::exception("Could not compile shader!");
	}

	return id;
}

// --------------------------------------------------------------------------------
void Shader::link(GLuint vertexShader, GLuint fragmentShader)
{

	GLint success = GL_FALSE;
	char debugLog[512];

	m_program = glCreateProgram();

	glAttachShader(m_program, vertexShader);
	glAttachShader(m_program, fragmentShader);
	glLinkProgram(m_program);

	glGetProgramiv(m_program, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(m_program, 512, nullptr, debugLog);
		throw std::exception("Could not link program!");
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

// --------------------------------------------------------------------------------
void Shader::use() const
{
	glUseProgram(m_program);
}