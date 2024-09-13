#include "Renderer.h"

#include "Framebuffer.h"

#include <glad/glad.h>
#include <glfw3.h>

#include "Vector3.h"
#include <math.h>
#include <vector>
#include <assert.h>

# define M_PI 3.14159265358979323846

// --------------------------------------------------------------------------------
Renderer::Renderer()
{

}

// --------------------------------------------------------------------------------
void Renderer::UpdateFramebufferContents(Framebuffer* framebuffer, uint32_t frame, bool& changeColour)
{
	float aspectRatio = (float)framebuffer->GetWidth() / (float)framebuffer->GetHeight();

	const float c_width = 2.0f;
	const float c_height = c_width / aspectRatio;

	const float halfFov = 45.0f * ((float)M_PI / 180.0f);
	const float distanceToPlane = (c_width / 2.0f) / tanf(halfFov);

	const float c_texelWidth = c_width /framebuffer->GetWidth();
	const float c_texelHeight = c_height / framebuffer->GetHeight();

	// Corners of the plane
	const Vector3 c_topLeftTexel(-(c_width / 2.0f), c_height / 2.0f, -distanceToPlane);
	const Vector3 c_bottomLeftTexel(-(c_width / 2.0f), -(c_height / 2.0f), -distanceToPlane);
	const Vector3 c_topRightTexel(c_width / 2.0f, c_height / 2.0f, -distanceToPlane);
	const Vector3 c_bottomRightTexel(c_width / 2.0f, -(c_height / 2.0f), -distanceToPlane);

	uint8_t* bytes = framebuffer->GetDataPtr();
	for (uint32_t row = 0u; row < framebuffer->GetHeight(); row++)
	{
		const float ty = (row * c_texelHeight) / c_height;
		const Vector3 c_r0 = (1.0f - ty) * c_topLeftTexel + ty * c_bottomLeftTexel;
		const Vector3 c_r1 = (1.0f - ty) * c_topRightTexel + ty * c_bottomRightTexel;

		for (uint32_t column = 0u; column < framebuffer->GetWidth(); column++)
		{
			const float tx = (column * c_texelWidth) / c_width;

			Vector3 texelCenter = (1.0f - tx) * c_r0 + tx * c_r1;
			texelCenter.SetX(texelCenter.X() + (c_texelWidth / 2.0f));
			texelCenter.SetY(texelCenter.Y() - (c_texelHeight / 2.0f));

			const uint32_t texelByteIndex = (row * framebuffer->GetWidth() * framebuffer->GetNumChannels()) + (column * framebuffer->GetNumChannels());
			assert(texelByteIndex < (framebuffer->GetWidth() * framebuffer->GetHeight() * framebuffer->GetNumChannels()));

			if (framebuffer->GetNumChannels() == 3u)
			{
				bytes[texelByteIndex] = 255u;
				bytes[texelByteIndex + 1u] = 0u;
				bytes[texelByteIndex + 2u] = 0u;
			}
			else if (framebuffer->GetNumChannels() == 4u)
			{
				bytes[texelByteIndex] = 255u;
				bytes[texelByteIndex + 1u] = 0u;
				bytes[texelByteIndex + 2u] = 0u;
				bytes[texelByteIndex + 3u] = 1u;
			}
		}
	}
}
