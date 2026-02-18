#pragma once

#include <string>
#include <glad/glad.h>

// --------------------------------------------------------------------------------
class Shader
{
public:

	Shader(const char* vertexShader, const char* fragmentShader);

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	std::string readSourceCode(const char* file) const;

	GLuint compile(const char* sourceCode, GLenum shaderStage) const;

	void link(GLuint vertexShader, GLuint fragmentShader);

	void use() const;

private:

	GLuint m_program;

};

