#include "Renderer.h"

#include "Framebuffer.h"

#include <glad/glad.h>
#include <glfw3.h>

#include "Vector3.h"
#include <math.h>
#include <vector>
#include <assert.h>
#include <cmath>
#include "Ray.h"

# define M_PI 3.14159265358979323846

// --------------------------------------------------------------------------------
Renderer::Renderer()
{
	m_sphereList.push_back(Vector3(0.0f, 0.4f, -2.0f)); // green, in front
	m_sphereList.push_back(Vector3(0.5f, 0.4f, -2.5f)); // blue, behind

	RGB c_green;
	c_green.m_red = 0u;
	c_green.m_green = 255u;
	c_green.m_blue = 0u;

	RGB c_blue;
	c_blue.m_red = 0u;
	c_blue.m_green = 0u;
	c_blue.m_blue = 255u;


	m_sphereColours.push_back(c_green);
	m_sphereColours.push_back(c_blue);

	assert(m_sphereList.size() == m_sphereColours.size());
}

// --------------------------------------------------------------------------------
void Renderer::UpdateFramebufferContents(Framebuffer* framebuffer)
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

			// Byte offsets
			const uint32_t texelByteIndex = (row * framebuffer->GetWidth() * framebuffer->GetNumChannels()) + (column * framebuffer->GetNumChannels());
			assert(texelByteIndex < (framebuffer->GetWidth() * framebuffer->GetHeight() * framebuffer->GetNumChannels()));
			
			const HitResult c_primaryHitResult = TraceRay(Vector3(0.0f, 1.0f, 0.0f), texelCenter, 1e-5f, INFINITY);
			if (c_primaryHitResult.m_t != INFINITY)
			{
				const Vector3 lightDirection = Normalize(Vector3(-1.0f, 1.0f, -1.0f));
				const float clampValue = std::fmin(std::fmax(Dot(lightDirection, c_primaryHitResult.m_normal), 0.0f), 1.0f);

				const HitResult c_secondaryRayHitResult = TraceRay(c_primaryHitResult.m_intersectionPoint, lightDirection, 1e-5f, INFINITY);

				if (c_secondaryRayHitResult.m_t == INFINITY)
				{
					bytes[texelByteIndex] = clampValue * c_primaryHitResult.m_colour.m_red;
					bytes[texelByteIndex + 1u] = clampValue * c_primaryHitResult.m_colour.m_green;
					bytes[texelByteIndex + 2u] = clampValue * c_primaryHitResult.m_colour.m_blue;
					bytes[texelByteIndex + 3u] = 1u;
				}
				else
				{
					bytes[texelByteIndex] = 0u;
					bytes[texelByteIndex + 1u] = 0u;
					bytes[texelByteIndex + 2u] = 0u;
					bytes[texelByteIndex + 3u] = 1u;
				}
			}
			else
			{
				bytes[texelByteIndex] = 255u;
				bytes[texelByteIndex + 1u] = 0u;
				bytes[texelByteIndex + 2u] = 0u;
				bytes[texelByteIndex + 3u] = 1u;
			}
		}
	}
}


// --------------------------------------------------------------------------------
void Renderer::HitSphere(const Vector3& origin, const Vector3& direction, const float tMin, const float tMax, HitResult& hitResult)
{
	// Calculate world space ray
	const Ray c_ray(origin, direction); // Will go in camera later

	const float sphereRadius = 0.4f;

	for (uint32_t sphere = 0u; sphere < m_sphereList.size(); sphere++)
	{
		const Vector3 rayOriginToSphere = m_sphereList[sphere] - c_ray.Origin();
		const float a = Dot(c_ray.Direction(), c_ray.Direction());
		const float b = -2.0f * Dot(c_ray.Direction(), rayOriginToSphere);
		const float c = Dot(rayOriginToSphere, rayOriginToSphere) - (sphereRadius * sphereRadius);

		// If an intersection has occurred
		const float discriminant = b * b - 4 * a * c;
		if (discriminant >= 0.0f)
		{
			// If the intersection is closer than previously stored distance
			const float t = (-b - sqrtf(discriminant)) / (2.0f * a);
			if (t < hitResult.m_t && t >= tMin && t <= tMax)
			{
				hitResult.m_t = t;
				hitResult.m_intersectionPoint = c_ray.CalculateIntersectionPoint(hitResult.m_t);
				hitResult.m_colour = m_sphereColours[sphere];
				hitResult.m_normal = Normalize(c_ray.CalculateIntersectionPoint(t) - m_sphereList[sphere]);
			}
		}
	}
}

// --------------------------------------------------------------------------------
void Renderer::HitPlane(const Vector3& origin, const Vector3& direction, const float tMin, const float tMax, HitResult& hitResult)
{
	// Calculate world space ray
	const Ray c_ray(origin, direction); // Will go in camera later, direction is normalized
	const float distance = 0.0f;

	const Vector3 planeNormal(0.0f, 1.0f, 0.0f); 
	const Vector3 normalizedPlaneNormal = Normalize(planeNormal);

	const float denom = Dot(normalizedPlaneNormal, c_ray.Direction());

	RGB c_indigo;
	c_indigo.m_red = 75u;
	c_indigo.m_green = 0u;
	c_indigo.m_blue = 130u;

	if (std::fabs(denom) >= 1e-8f)
	{
		const float t = (distance - Dot(normalizedPlaneNormal, c_ray.Origin())) / denom;

		if (t >= 0.0f && t < hitResult.m_t && t >= tMin && t <= tMax)
		{
			hitResult.m_t = t;
			hitResult.m_intersectionPoint = c_ray.CalculateIntersectionPoint(hitResult.m_t);
			hitResult.m_colour = c_indigo;
			hitResult.m_normal = normalizedPlaneNormal;
		}
	}
}

// --------------------------------------------------------------------------------
HitResult Renderer::TraceRay(const Vector3& origin, const Vector3& direction, const float tMin, const float tMax)
{
	HitResult hitResult;

	HitSphere(origin, direction, tMin, tMax, hitResult);
	HitPlane(origin, direction, tMin, tMax, hitResult);

	return hitResult;
}
