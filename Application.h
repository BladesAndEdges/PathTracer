#pragma once

class GLContext;
class Renderer;

class Application
{
public:

	Application();

	Application(Application&) = delete;
	Application& operator=(const Application&) = delete;

	~Application();

	void Run();

private:

	GLContext* m_context;
	Renderer* m_renderer;
};

