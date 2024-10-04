#include "Renderer.h"

#include "Framebuffer.h"

#include <glad/glad.h>
#include <glfw3.h>

#include "Vector3.h"
#include <math.h>
#include <vector>
#include <assert.h>
#include <cmath>
#include "PerformanceCounter.h"

# define M_PI 3.14159265358979323846

// --------------------------------------------------------------------------------
Renderer::Renderer()
{
	RGB c_green;
	c_green.m_red = 0u;
	c_green.m_green = 255u;
	c_green.m_blue = 0u;

	RGB c_blue;
	c_blue.m_red = 0u;
	c_blue.m_green = 0u;
	c_blue.m_blue = 255u;

	for (int i = 0; i < 10u; i++)
	{
		for (int j = 0; j < 10u; j++)
		{
			const Vector3 bottomLeft(-5.0f, 0.4f, -2.0f);
			const Vector3 position(bottomLeft.X() + i, bottomLeft.Y(), bottomLeft.Z() - j);
			m_sphereList.push_back(position);

			if ((i % 2) == 0)
			{
				m_sphereColours.push_back(c_green);
			}
			else
			{
				m_sphereColours.push_back(c_blue);
			}
		}
	}

	assert(m_sphereList.size() == m_sphereColours.size());

	m_camera.SetCameraLocation(Vector3(0.0f, 1.0f, 0.0f));
	m_lightDirection = Normalize(Vector3(-1.0f, 1.0f, -1.0f));

	ZeroMemory((void*)&m_viewportDesc, sizeof(m_viewportDesc)); // ?
	m_isFirstFrame = true;
}

// --------------------------------------------------------------------------------
Camera* Renderer::GetCamera()
{
	return &m_camera;
}

// --------------------------------------------------------------------------------
void Renderer::UpdateFramebufferContents(Framebuffer* framebuffer, bool hasResized)
{
	if (hasResized || m_isFirstFrame)
	{
		m_texelCenters.clear();

		m_viewportDesc.m_aspectRatio = (float)framebuffer->GetWidth() / (float)framebuffer->GetHeight();
		m_viewportDesc.m_width = 2.0f;
		m_viewportDesc.m_height = m_viewportDesc.m_width / m_viewportDesc.m_aspectRatio;
		m_viewportDesc.m_halfFov = 45.0f * ((float)M_PI / 180.0f);
		m_viewportDesc.m_distanceToPlane = ((m_viewportDesc.m_width / 2.0f) / tanf(m_viewportDesc.m_halfFov));
		m_viewportDesc.m_texelWidth = m_viewportDesc.m_width / framebuffer->GetWidth();
		m_viewportDesc.m_texelHeight = m_viewportDesc.m_height / framebuffer->GetHeight();
		// Corners of the plane
		m_viewportDesc.m_topLeftTexel = Vector3(-(m_viewportDesc.m_width / 2.0f), m_viewportDesc.m_height / 2.0f, -m_viewportDesc.m_distanceToPlane);
		m_viewportDesc.m_bottomLeftTexel = Vector3(-(m_viewportDesc.m_width / 2.0f), -(m_viewportDesc.m_height / 2.0f), -m_viewportDesc.m_distanceToPlane);
		m_viewportDesc.m_topRightTexel = Vector3(m_viewportDesc.m_width / 2.0f, m_viewportDesc.m_height / 2.0f, -m_viewportDesc.m_distanceToPlane);
		m_viewportDesc.m_bottomRightTexel = Vector3(m_viewportDesc.m_width / 2.0f, -(m_viewportDesc.m_height / 2.0f), -m_viewportDesc.m_distanceToPlane);

		uint8_t* bytes = framebuffer->GetDataPtr();
		for (uint32_t row = 0u; row < framebuffer->GetHeight(); row++)
		{
			const float ty = (row * m_viewportDesc.m_texelHeight) / m_viewportDesc.m_height;
			const Vector3 c_r0 = (1.0f - ty) * m_viewportDesc.m_topLeftTexel + ty * m_viewportDesc.m_bottomLeftTexel;
			const Vector3 c_r1 = (1.0f - ty) * m_viewportDesc.m_topRightTexel + ty * m_viewportDesc.m_bottomRightTexel;

			for (uint32_t column = 0u; column < framebuffer->GetWidth(); column++)
			{
				const float tx = (column * m_viewportDesc.m_texelWidth) / m_viewportDesc.m_width;

				Vector3 texelCenter = (1.0f - tx) * c_r0 + tx * c_r1;
				texelCenter.SetX(texelCenter.X() + (m_viewportDesc.m_texelWidth / 2.0f));
				texelCenter.SetY(texelCenter.Y() - (m_viewportDesc.m_texelHeight / 2.0f));
				m_texelCenters.push_back(texelCenter);
			}
		}
		
		m_isFirstFrame = false;
	}

	//HitResult c_primaryHitResult;
	//HitResult c_secondaryRayHitResult;
	uint8_t* bytes = framebuffer->GetDataPtr();
	for (uint32_t row = 0u; row < framebuffer->GetHeight(); row++)
	{
		for (uint32_t column = 0u; column < framebuffer->GetWidth(); column++)
		{
			const uint32_t index = (row * framebuffer->GetWidth() + column);

			// Byte offsets
			const uint32_t texelByteIndex = (row * framebuffer->GetWidth() * framebuffer->GetNumChannels()) + (column * framebuffer->GetNumChannels());
			assert(texelByteIndex < (framebuffer->GetWidth() * framebuffer->GetHeight() * framebuffer->GetNumChannels()));
			
			const Ray c_primaryRay(m_camera.GetCameraLocation(), m_texelCenters[index]);
			const HitResult c_primaryHitResult = TraceRay(c_primaryRay, 1e-5f, 50.0f);
			//TraceRayRef(c_primaryRay, 1e-5f, INFINITY, c_primaryHitResult);
			if (c_primaryHitResult.m_t != INFINITY)
			{
				const float clampValue = std::fmin(std::fmax(Dot(m_lightDirection, c_primaryHitResult.m_normal), 0.0f), 1.0f);

				const Ray c_shadowRay(c_primaryHitResult.m_intersectionPoint, m_lightDirection);
				const HitResult c_secondaryRayHitResult = TraceRay(c_shadowRay, 1e-5f, 50.0f);
				//TraceRayRef(c_shadowRay, 1e-5f, INFINITY, c_secondaryRayHitResult);

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

			//c_primaryHitResult.m_t = INFINITY;
			//c_secondaryRayHitResult.m_t = INFINITY;
		}
	}
}

// --------------------------------------------------------------------------------
void Renderer::HitSphere(const Ray& ray, const float tMin, const float tMax, HitResult& out_hitResult)
{
	const float sphereRadius = 0.4f;
	const float a = Dot(ray.Direction(), ray.Direction());

	for (uint32_t sphere = 0u; sphere < m_sphereList.size(); sphere++)
	{
		const Vector3 rayOriginToSphere = m_sphereList[sphere] - ray.Origin();
		const float b = -2.0f * Dot(ray.Direction(), rayOriginToSphere);
		const float c = Dot(rayOriginToSphere, rayOriginToSphere) - (sphereRadius * sphereRadius);

		// If an intersection has occurred
		const float discriminant = b * b - 4.0f * a * c;
		if (discriminant >= 0.0f)
		{
			// If the intersection is closer than previously stored distance
			const float t = (-b - sqrtf(discriminant)) / (2.0f * a);
			if (t < out_hitResult.m_t && t >= tMin && t <= tMax)
			{
				out_hitResult.m_t = t;
				out_hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(out_hitResult.m_t);
				out_hitResult.m_colour = m_sphereColours[sphere];
				out_hitResult.m_normal = Normalize(ray.CalculateIntersectionPoint(t) - m_sphereList[sphere]);
			}
		}
	}
}

// --------------------------------------------------------------------------------
void Renderer::HitPlane(const Ray& ray, const float tMin, const float tMax, const float distance, const Vector3& normalizedPlaneNormal, RGB colour, HitResult& out_hitResult)
{
	const float denom = Dot(normalizedPlaneNormal, ray.Direction());

	if (std::fabs(denom) >= 1e-8f)
	{
		const float t = (distance - Dot(normalizedPlaneNormal, ray.Origin())) / denom;

		if (t < out_hitResult.m_t && t >= tMin && t <= tMax)
		{
			out_hitResult.m_t = t;
			out_hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(out_hitResult.m_t);
			out_hitResult.m_colour = colour;
			out_hitResult.m_normal = normalizedPlaneNormal;
		}
	}
}

// --------------------------------------------------------------------------------
HitResult Renderer::TraceRay(const Ray& ray, const float tMin, const float tMax)
{
	HitResult hitResult;

	RGB c_indigo;
	c_indigo.m_red = 75u;
	c_indigo.m_green = 0u;
	c_indigo.m_blue = 130u;

	HitSphere(ray, tMin, tMax, hitResult);
	HitPlane(ray, tMin, tMax, 0.0f, Normalize(Vector3(0.0f, 1.0f, 0.0f)), c_indigo, hitResult);

	return hitResult;
}
