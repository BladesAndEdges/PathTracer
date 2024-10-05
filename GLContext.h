#pragma once

#include<Windows.h>
#include<glad/glad.h>
#include<glfw3.h>

class Framebuffer;
class Shader;
class Camera;

class GLContext
{
public:

	GLContext();
	GLContext(const GLContext&) = delete;
	GLContext& operator=(const GLContext&) = delete;

	~GLContext();

	int ShouldClose();
	void Listen();
	bool HasFramebufferChanged();
	void ResizeFramebuffer();
	void UpdateFramebuffer();
	void UpdatePerformanceStatistics(double milliseconds);
	void ProcessCameraInput(Camera* camera);
	void Draw();
	void SwapBuffers();

	Framebuffer* GetFramebuffer() const;

private:

	void InitializeGLFW();
	void CreateGLFWWindow();
	void InitializeGlad();
	void RegisterDebugCallback();

	GLFWwindow* m_glfwWindow;
	Framebuffer* m_framebuffer;
	Shader* m_shader;
	GLuint m_vao;
};

// --------------------------------------------------------------------------------
void GLAPIENTRY DebugMessageCallback(GLenum,
	GLenum,
	GLuint,
	GLenum severity,
	GLsizei,
	const GLchar* message,
	const void*);
