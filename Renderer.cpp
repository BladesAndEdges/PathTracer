#include "Renderer.h"

#include "Framebuffer.h"

#include <glad/glad.h>

// --------------------------------------------------------------------------------
Renderer::Renderer()
{

}

// --------------------------------------------------------------------------------
void Renderer::UpdateFramebufferContents(Framebuffer* framebuffer, uint32_t frame, bool& changeColour)
{
	uint32_t width = framebuffer->GetWidth();
	uint32_t height = framebuffer->GetHeight();
	uint32_t numChannels = framebuffer->GetNumChannels();
	uint8_t* bytes = framebuffer->GetDataPtr();

	if ((frame % 100) == 0)
	{
		changeColour = !changeColour;
	}

	for (uint32_t row = 0; row < height; row++)
	{
		for (uint32_t column = 0; column < width; column++)
		{
			for (int byte = 0; byte < 3; byte++)
			{
				if (changeColour)
				{
					if (byte == 0)
					{
						bytes[(row * width + column) * numChannels + byte] = (GLubyte)255;
					}
					else
					{
						bytes[(row * width + column) * numChannels + byte] = (GLubyte)0;
					}
				}
				else
				{
					if (byte == 1)
					{
						bytes[(row * width + column) * numChannels + byte] = (GLubyte)255;
					}
					else
					{
						bytes[(row * width + column) * numChannels + byte] = (GLubyte)0;
					}
				}
			}
		}
	}
}
