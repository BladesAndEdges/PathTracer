#include "Application.h"

#include "GLContext.h"
#include "PerformanceCounter.h"
#include "Renderer.h"

#define ArraySize(x) sizeof(x)/sizeof(x[0]);

// --------------------------------------------------------------------------------
Application::Application()
{
	m_context = new GLContext();
	m_renderer = new Renderer();

	ZeroMemory(m_frameTimes, 256u * sizeof(double));
}

// --------------------------------------------------------------------------------
Application::~Application()
{
	delete m_renderer;
	delete m_context;
}

// --------------------------------------------------------------------------------
void Application::Run()
{
	PerformanceCounter counter;
	bool test = false;
	uint64_t frameNumber = 0u;

	// Rendering and all that 
	while (!m_context->ShouldClose())
	{
		counter.BeginTiming();
		m_context->Listen();

		const double c_averageTime = CalculateAverageFrameTime(counter.GetMilliseconds(), frameNumber, m_frameTimes);
		m_context->UpdatePerformanceStatistics(c_averageTime);

		m_context->ProcessCameraInput(m_renderer->GetCamera());

		if (test = m_context->HasFramebufferChanged())
		{
			m_context->ResizeFramebuffer();
		}

		m_renderer->UpdateFramebufferContents(m_context->GetFramebuffer(), test);
		m_context->UpdateFramebuffer();

		m_context->Draw();

		m_context->SwapBuffers();
		counter.EndTiming();

		frameNumber++;
	}
}

// --------------------------------------------------------------------------------
double CalculateAverageFrameTime(const double frameTimeInMilisecods, unsigned int frameNumber, double timings[128])
{
	const unsigned int arraySize = ArraySize(timings);
	const int indexInArray = frameNumber % arraySize;
	timings[indexInArray] = frameTimeInMilisecods;

	double sum = 0.0f;
	if (frameNumber >= 128)
	{
		for (unsigned int frame = 0; frame < arraySize; frame++)
		{
			sum += timings[frame];
		}
	}

	const double averageTime = sum / arraySize;
	return averageTime;
}
