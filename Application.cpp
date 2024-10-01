#include "Application.h"

#include "GLContext.h"
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
	// Rendering and all that 
	while (!m_context->ShouldClose())
	{
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
	}
}
