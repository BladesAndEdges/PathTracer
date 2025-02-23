#include "Application.h"

#include <assert.h>

#include "GLContext.h"
#include "ModelParser.h"
#include "PerformanceCounter.h"
#include "Renderer.h"

// --------------------------------------------------------------------------------
Application::Application()
{
	m_parser = new ModelParser();
	m_parser->ParseFile(R"(Scenes\CornellBox\CornellBox-Empty-CO.obj)", 1.0f);

	m_context = new GLContext();
	m_renderer = new Renderer(m_parser->GetPositionsX(), m_parser->GetPositionsY(), m_parser->GetPositionsZ(), 
		m_parser->GetTriangle4Data(), m_parser->GetFaces(), m_parser->GetCenter()); // Model will hold the data eventually once I add it

	ZeroMemory(m_frameTimes, FRAME_TIMINGS_BUFFER_SIZE * sizeof(double));
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
	bool hasResized = false;
	uint32_t frameNumber = 0u;

	// Rendering and all that 
	while (!m_context->ShouldClose())
	{
		counter.BeginTiming();
		m_context->Listen();

		const double c_averageTime = CalculateAverageFrameTime(counter.GetMilliseconds(), frameNumber, m_frameTimes);
		m_context->UpdatePerformanceStatistics(c_averageTime);

		m_context->ProcessCameraInput(m_renderer->GetCamera());

		if ((hasResized = m_context->HasFramebufferChanged()))
		{
			m_context->ResizeFramebuffer();
		}

		m_renderer->UpdateFramebufferContents(m_context->GetFramebuffer(), hasResized);
		m_context->UpdateFramebuffer();

		m_context->Draw();

		m_context->SwapBuffers();
		counter.EndTiming();

		frameNumber++;
	}
}
#include <assert.h>

// --------------------------------------------------------------------------------
double CalculateAverageFrameTime(const double frameTimeInMilisecods, uint32_t frameNumber, double buffer[FRAME_TIMINGS_BUFFER_SIZE])
{
	const uint32_t indexInArray = frameNumber % FRAME_TIMINGS_BUFFER_SIZE;
	buffer[indexInArray] = frameTimeInMilisecods;

	const uint32_t currentFrameCount = (frameNumber >= FRAME_TIMINGS_BUFFER_SIZE) ? FRAME_TIMINGS_BUFFER_SIZE : frameNumber + 1u;

	double sum = 0.0f;
	for (uint32_t frame = 0; frame < currentFrameCount; frame++)
	{
		sum += buffer[frame];
	}

	const double averageTime = sum / currentFrameCount;
	return averageTime;
}
