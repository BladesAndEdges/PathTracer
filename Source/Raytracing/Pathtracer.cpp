#include "Pathtracer.h"

# define M_PI 3.14159265358979323846

// --------------------------------------------------------------------------------
Pathtracer::Pathtracer()
{
}

// --------------------------------------------------------------------------------
void Pathtracer::CreateImage()
{
}

// --------------------------------------------------------------------------------
void Pathtracer::GenerateViewspaceDirections(const uint32_t framebufferWidth, const uint32_t framebufferHeight)
{
	m_viewspaceDirections.clear();

	const float aspectRatio = (float)framebufferWidth / (float)framebufferHeight;
	const float width = 2.0f;
	const float height = width / aspectRatio;
	const float halfFov = 45.0f * ((float)M_PI / 180.0f);
	const float distanceToPlane = (width / 2.0f) / tanf(halfFov);

	m_pixelWidth = width / framebufferWidth;
	m_pixelHeight = height / framebufferHeight;

	// Corners of the plane
	const Vector3 topLeft(-width / 2.0f, height / 2.0f, -distanceToPlane);
	const Vector3 bottomLeft(-width / 2.0f, -height / 2.0f, -distanceToPlane);
	const Vector3 topRight(width / 2.0f, height / 2.0f, -distanceToPlane);
	const Vector3 bottomRight(width / 2.0f, -height / 2.0f, -distanceToPlane);

	for (uint32_t row = 0u; row < framebufferHeight; row++)
	{
		const float ty = (row * m_pixelHeight) / height;
		const Vector3 r0 = (1.0f - ty) * topLeft + ty * bottomLeft;
		const Vector3 r1 = (1.0f - ty) * topRight + ty * bottomRight;

		for (uint32_t column = 0u; column < framebufferWidth; column++)
		{
			const float tx = (column * m_pixelWidth) / width;

			Vector3 texelCenter = (1.0f - tx) * r0 + tx * r1;
			const float xVal = texelCenter.X() + (m_pixelWidth / 2.0f);

			const float valDegreesToRad = 90.0f * ((float)M_PI / 180.0f);
			const float rotatedX = cosf(valDegreesToRad) * xVal + sinf(valDegreesToRad) * texelCenter.Z();
			const float rotatedZ = -sinf(valDegreesToRad) * xVal + cosf(valDegreesToRad) * texelCenter.Z();

			texelCenter.SetX(rotatedX);
			texelCenter.SetY(texelCenter.Y() - (m_pixelHeight / 2.0f));
			texelCenter.SetZ(rotatedZ);
			m_viewspaceDirections.push_back(texelCenter);
		}
	}
}

// --------------------------------------------------------------------------------
const std::vector<Vector3>& Pathtracer::GetViewSpaceDirections() const
{
	return m_viewspaceDirections;
}

// --------------------------------------------------------------------------------
float Pathtracer::GetPixelWidth() const
{
	return m_pixelWidth;
}

// --------------------------------------------------------------------------------
float Pathtracer::GetPixelHeight() const
{
	return m_pixelHeight;
}
