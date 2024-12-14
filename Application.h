#pragma once

#include<stdint.h>

#define FRAME_TIMINGS_BUFFER_SIZE 256

class GLContext;
class ModelParser;
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
	ModelParser* m_parser;
	Renderer* m_renderer;

	double m_frameTimes[FRAME_TIMINGS_BUFFER_SIZE];
};

double CalculateAverageFrameTime(const double frameTimeInMilisecods, uint32_t frameNumber, double buffer[FRAME_TIMINGS_BUFFER_SIZE]);
