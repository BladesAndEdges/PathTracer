#include "GLContext.h"

#include<exception>

#include "Camera.h"
#include "Framebuffer.h"
#include "Shader.h"
#include "Vector3.h"

#define NUMBER_OF_CHANNELS_PER_TEXEL 4u

float g_deltaTime = 0.0f;
float g_previousFrameTime = 0.0f;

// --------------------------------------------------------------------------------
GLContext::GLContext()
{
	InitializeGLFW();

	CreateGLFWWindow();

	InitializeGlad();

	RegisterDebugCallback();

	glCreateVertexArrays(1, &m_vao);

	m_shader = new Shader("Shaders/MainShader.vert", "Shaders/MainShader.frag");

	int framebufferWidth, framebufferHeight;
	glfwGetFramebufferSize(m_glfwWindow, &framebufferWidth, &framebufferHeight);
	m_framebuffer = new Framebuffer(framebufferWidth, framebufferHeight, NUMBER_OF_CHANNELS_PER_TEXEL);
}

// --------------------------------------------------------------------------------
GLContext::~GLContext()
{
	delete m_framebuffer;

	delete m_shader;

	glDeleteVertexArrays(1, &m_vao);

	glfwDestroyWindow(m_glfwWindow);

	glfwTerminate();
}

// --------------------------------------------------------------------------------
int GLContext::ShouldClose()
{
	return glfwWindowShouldClose(m_glfwWindow);
}

// --------------------------------------------------------------------------------
void GLContext::Listen()
{
	glfwPollEvents();
}

// --------------------------------------------------------------------------------
bool GLContext::HasFramebufferChanged()
{
	int currentWidth, currentHeight;
	glfwGetFramebufferSize(m_glfwWindow, &currentWidth, &currentHeight);
	return (m_framebuffer->GetWidth() != (uint32_t)currentWidth) || (m_framebuffer->GetHeight() != (uint32_t)currentHeight);
}

// --------------------------------------------------------------------------------
void GLContext::ResizeFramebuffer()
{
	delete m_framebuffer; // Any synchronization needed here?

	int currentWidth, currentHeight;
	glfwGetFramebufferSize(m_glfwWindow, &currentWidth, &currentHeight);
	m_framebuffer = new Framebuffer(currentWidth, currentHeight, NUMBER_OF_CHANNELS_PER_TEXEL);
}

// --------------------------------------------------------------------------------
void GLContext::UpdateFramebuffer()
{
	m_framebuffer->Update();
}

// --------------------------------------------------------------------------------
void GLContext::ProcessCameraInput(Camera* camera)
{
	float currentFrameTime = (float)glfwGetTime();
	g_deltaTime = currentFrameTime - g_previousFrameTime;
	g_previousFrameTime = currentFrameTime;

	const float unitsPerFrame = 1.5f * g_deltaTime;

	const Vector3 forward(0.0f, 0.0f, 1.0f);
	const Vector3 up(0.0f, 1.0f, 0.0f);
	const Vector3 right(1.0f, 0.0f, 0.0f);

	//Camera Translation
	Vector3 viewSpaceTranslation(0.0f, 0.0f, 0.0f);

	if (glfwGetKey(m_glfwWindow, GLFW_KEY_W) == GLFW_PRESS)
	{
		viewSpaceTranslation = viewSpaceTranslation - (unitsPerFrame * forward);
	}

	if (glfwGetKey(m_glfwWindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		viewSpaceTranslation = viewSpaceTranslation + (unitsPerFrame * forward);
	}

	if (glfwGetKey(m_glfwWindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		viewSpaceTranslation = viewSpaceTranslation - (unitsPerFrame * right);
	}

	if (glfwGetKey(m_glfwWindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		viewSpaceTranslation = viewSpaceTranslation + (unitsPerFrame * right);
	}

	if (glfwGetKey(m_glfwWindow, GLFW_KEY_Q) == GLFW_PRESS)
	{
		viewSpaceTranslation = viewSpaceTranslation + (unitsPerFrame * up);
	}

	if (glfwGetKey(m_glfwWindow, GLFW_KEY_E) == GLFW_PRESS)
	{
		viewSpaceTranslation = viewSpaceTranslation - (unitsPerFrame * up);
	}

	const Vector3 c_worldSpaceTranslation = /*camera.getWorldOrientation() **/ viewSpaceTranslation;
	const Vector3 c_worldSpacePosition = camera->GetCameraLocation() + c_worldSpaceTranslation;
	camera->SetCameraLocation(c_worldSpacePosition);
}

// --------------------------------------------------------------------------------
void GLContext::Draw()
{
	glViewport(0, 0, m_framebuffer->GetWidth(), m_framebuffer->GetHeight());
	glBindVertexArray(m_vao);
	m_framebuffer->Use();
	m_shader->use();
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

// --------------------------------------------------------------------------------
void GLContext::SwapBuffers()
{
	glfwSwapBuffers(m_glfwWindow);
}

// --------------------------------------------------------------------------------
Framebuffer* GLContext::GetFramebuffer() const
{
	return m_framebuffer;
}

// --------------------------------------------------------------------------------
void GLContext::InitializeGLFW()
{
	if (!glfwInit())
	{
		throw std::exception("Unable to initialize GLFW.");
	}
}

// --------------------------------------------------------------------------------
void GLContext::CreateGLFWWindow()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	m_glfwWindow = glfwCreateWindow(1024u, 720u, "Path Tracer", nullptr, nullptr);

	glfwMakeContextCurrent(m_glfwWindow);
}

// --------------------------------------------------------------------------------
void GLContext::InitializeGlad()
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw std::exception("Could not load OpenGL function loader.");
	}
}

// --------------------------------------------------------------------------------
void GLContext::RegisterDebugCallback()
{
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DebugMessageCallback, 0);
}

// --------------------------------------------------------------------------------
void GLAPIENTRY DebugMessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void*)
{
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		OutputDebugStringA(message);
		__debugbreak();
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		OutputDebugStringA(message);
		__debugbreak();
		break;
	case GL_DEBUG_SEVERITY_LOW:
		OutputDebugStringA(message);
		__debugbreak();
		break;
	default:
		break;
	}
}
