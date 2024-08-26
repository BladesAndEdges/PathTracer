#include<Windows.h>
#include<exception>

#include<imgui.h>

#include<glad/glad.h>
#include<glfw3.h>

#include "Framebuffer.h"
#include "Shader.h"

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

// --------------------------------------------------------------------------------
void GenerateFrameData( Framebuffer* framebuffer,const uint32_t frame, bool& changeColour)
{
	if ((frame % 100) == 0)
	{
		changeColour = !changeColour;
	}

	const uint32_t width = framebuffer->getWidth();
	const uint32_t height = framebuffer->getHeight();
	const uint32_t channelsPerTexel = framebuffer->getChannels();
	uint8_t* bytes = framebuffer->getData();

	for (uint32_t row = 0; row < height; row++)
	{
		for (uint32_t column = 0; column < width; column++)
		{
			for (int byte = 0; byte < 3; byte++)
			{
				if ( changeColour )
				{
					if (byte == 0)
					{	
						bytes[(row * width + column) * channelsPerTexel + byte] = (GLubyte)255;
					}
					else
					{
						bytes[(row * width + column) * channelsPerTexel + byte] = (GLubyte)0;
					}
				}
				else
				{
					if (byte == 1)
					{
						bytes[(row * width + column) * channelsPerTexel + byte] = (GLubyte)255;
					}
					else
					{
						bytes[(row * width + column) * channelsPerTexel + byte] = (GLubyte)0;
					}
				}
			}
		}
	}
}

// --------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	if (!glfwInit())
	{
		throw std::exception("Unable to initialize GLFW.");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = nullptr;
	window = glfwCreateWindow(1024, 720, "Path Tracer", nullptr, nullptr);

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw std::exception("Could not load OpenGL function loader.");
	}

	// During init, enable debug output
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DebugMessageCallback, 0);

	Shader mainShader("Shaders/MainShader.vert", "Shaders/MainShader.frag");

	GLuint vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	Framebuffer* framebuffer = new Framebuffer(width, height, 3u);

	uint32_t frame = 0u;
	bool changeColour = false;
	
	// Rendering and all that 
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		if ((framebuffer->getWidth() != width) || (framebuffer->getHeight() != height))
		{
			delete framebuffer;
			framebuffer = new Framebuffer(width, height, 3u);
		}

		GenerateFrameData(framebuffer, frame, changeColour);
		framebuffer->update();

		framebuffer->use();

		glViewport(0, 0, width, height);
		mainShader.use();
		glDrawArrays(GL_TRIANGLES, 0, 3);


		glfwSwapBuffers(window);

		frame++;
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}