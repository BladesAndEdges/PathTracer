#include "Application.h"

#include "GLContext.h"
#include "PerformanceCounter.h"
#include "Renderer.h"

// --------------------------------------------------------------------------------
Application::Application()
{
	m_context = new GLContext();
	m_renderer = new Renderer();
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

	// Rendering and all that 
	while (!m_context->ShouldClose())
	{
		counter.BeginTiming();
		m_context->UpdatePerformanceStatistics(counter.GetMilliseconds());
		m_context->Listen();
		m_context->ProcessCameraInput(m_renderer->GetCamera());

		if (m_context->HasFramebufferChanged())
		{
			m_context->ResizeFramebuffer();
		}

		m_renderer->UpdateFramebufferContents(m_context->GetFramebuffer());
		m_context->UpdateFramebuffer();

		m_context->Draw();

		m_context->SwapBuffers();
		counter.EndTiming();
	}
}
