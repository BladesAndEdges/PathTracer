#include<Windows.h>
#include<exception>


#include<imgui.h>

#include<glad/glad.h>
#include<glfw3.h>

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
void GenerateFrameData(const uint32_t framebufferWidth, const uint32_t framebufferHeight, const uint32_t channelsPerTexel, const uint32_t frame, bool& changeColour, uint32_t& lastFrameWidth, uint32_t lastFrameHeight, GLubyte** bytes)
{
	if ((frame % 100) == 0)
	{
		changeColour = !changeColour;
	}

	// Handle memory allocation of the frame resource only when a resize occurs
	if ((lastFrameWidth != framebufferWidth) || (lastFrameHeight != framebufferHeight))
	{
		delete *bytes;
		*bytes = new GLubyte[framebufferWidth * framebufferHeight * channelsPerTexel];

		lastFrameWidth = framebufferWidth;
		lastFrameHeight = framebufferHeight;
	}

	for (uint32_t row = 0; row < framebufferHeight; row++)
	{
		for (uint32_t column = 0; column < framebufferWidth; column++)
		{
			for (int byte = 0; byte < 3; byte++)
			{
				if ( changeColour )
				{
					if (byte == 0)
					{	
						(*bytes)[(row * framebufferWidth + column) * channelsPerTexel + byte] = (GLubyte)255;
					}
					else
					{
						(*bytes)[(row * framebufferWidth + column) * channelsPerTexel + byte] = (GLubyte)0;
					}
				}
				else
				{
					if (byte == 1)
					{
						(*bytes)[(row * framebufferWidth + column) * channelsPerTexel + byte] = (GLubyte)255;
					}
					else
					{
						(*bytes)[(row * framebufferWidth + column) * channelsPerTexel + byte] = (GLubyte)0;
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

	const uint32_t width = 1024u;
	const uint32_t height = 720u;
	const uint32_t channelsPerTexel = 3u;

	uint32_t lastFrameWidth = 0u;
	uint32_t lastFrameHeight = 0u;
	GLubyte* bytes = nullptr;
	
	GLuint mainTexture;
	glCreateTextures(GL_TEXTURE_2D, 1, &mainTexture);

	glTextureParameteri(mainTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(mainTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteri(mainTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(mainTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTextureStorage2D(mainTexture, 1, GL_RGB8, width, height);

	uint32_t frame = 0u;
	bool changeColour = false;
	
	// Rendering and all that 
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		glBindTextureUnit(0, mainTexture);
		GenerateFrameData(width, height, channelsPerTexel, frame, changeColour, lastFrameWidth, lastFrameHeight, &bytes);
		glTextureSubImage2D(mainTexture, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, bytes); // Can update with this, but not resize as it is using immutable
																									// texture storage

		mainShader.use();
		glDrawArrays(GL_TRIANGLES, 0, 3);


		glfwSwapBuffers(window);

		frame++;
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}