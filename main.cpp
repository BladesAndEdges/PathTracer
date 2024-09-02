#include<imgui.h>

#include "GLContext.h"
#include "Renderer.h"

// --------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	GLContext* context = new GLContext();
	Renderer* renderer = new Renderer();

	uint32_t frame = 0u;
	bool changeColour = false;
	
	// Rendering and all that 
	while (!context->ShouldClose())
	{
		context->Listen();

		if (context->HasFramebufferChanged())
		{
			context->ResizeFramebuffer();
		}

		renderer->UpdateFramebufferContents(context->GetFramebuffer(), frame, changeColour);
		context->UpdateFramebuffer();

		context->Draw();

		context->SwapBuffers();

		frame++;
	}

	return 0;
}