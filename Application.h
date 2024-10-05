#pragma once

#include<stdint.h>

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

	double m_frameTimes[256u];
};

double CalculateAverageFrameTime(const double frameTimeInMilisecods, uint64_t frameNumber, double frameTimes[256u]);
