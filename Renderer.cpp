#include "Renderer.h"

#define NOMINMAX

#include <assert.h>
#include <emmintrin.h>
#include <float.h>
#include <immintrin.h>

#include "BVH2AccellStructure.h"
#include "BVH4AccellStructure.h"
#include "Framebuffer.h"
#include "Intersections.h"
#include "Material4Index.h"
#include "PerformanceCounter.h"
#include "SceneManager.h"
#include "TraversalDataManager.h"
#include "TraversalTriangle.h"
#include "TriangleTexCoords.h"
#include "TriangleTexCoords4.h"

# define M_PI 3.14159265358979323846
//#define TRACE_AGAINST_NON_BVH
#define TRACE_AGAINST_NON_BVH_SSE
//#define TRACE_AGAINST_BVH2
//#define TRACE_AGAINST_BVH4

// --------------------------------------------------------------------------------
Renderer::Renderer()
{
	ZeroMemory((void*)&m_viewportDesc, sizeof(m_viewportDesc));
	m_isFirstFrame = true;

	m_sceneManager = new SceneManager("sponza.obj", "sponza.mtl");

	m_traversalDataManager = new TraversalDataManager(m_sceneManager->GetTriangles(), m_sceneManager->GetPerTriangleMaterials());

	m_camera.SetCameraLocation(m_sceneManager->GetInitialCameraPosition());
	m_lightDirection = Normalize(Vector3(1.0f, 1.0f, 1.0f));
}

// --------------------------------------------------------------------------------
Camera* Renderer::GetCamera()
{
	return &m_camera;
}

// --------------------------------------------------------------------------------
void Renderer::UpdateFramebufferContents(Framebuffer* framebuffer, bool hasResized, PerformanceCounter& pc)
{
	if (hasResized || m_isFirstFrame)
	{
		RegenerateViewSpaceDirections(framebuffer);
	}

	std::vector<uint32_t> primaryRayAABBIntersectionsCount;
	std::vector<uint32_t> primaryRayTriangleIntersectionsCount;
	std::vector<uint32_t> primaryRayNodeVisits;

	//std::vector<uint32_t> primaryRayAABBIntersectionsCount;
	//std::vector<uint32_t> primaryRayTriangleIntersectionsCount;
	//std::vector<uint32_t> primaryRayNodeVisits;

	primaryRayAABBIntersectionsCount.resize(framebuffer->GetWidth() * framebuffer->GetHeight());
	primaryRayTriangleIntersectionsCount.resize(framebuffer->GetWidth() * framebuffer->GetHeight());
	primaryRayNodeVisits.resize(framebuffer->GetWidth() * framebuffer->GetHeight());

	const SHORT cKeyState = GetAsyncKeyState(0x43);
	const SHORT vKeyState = GetAsyncKeyState(0x56);
	const SHORT bKeyState = GetAsyncKeyState(0x42);
	const SHORT nKeyState = GetAsyncKeyState(0x4E);
	const SHORT mKeyState = GetAsyncKeyState(0x4D);
	const SHORT xKeyState = GetAsyncKeyState(0x58);
	const SHORT lKeyState = GetAsyncKeyState(0x4C);
	const SHORT kKeyState = GetAsyncKeyState(0x4B);

	const Vector3 primitiveDebugColours[5u] = { Vector3(0.94f, 0.34f, 0.30f), Vector3(0.30f, 0.94f, 0.70f), Vector3(0.51f, 0.70f, 0.96f),
		Vector3(0.96f, 0.91f, 0.51f), Vector3(0.96f, 0.61f, 0.91f) };

	//----------------------------------------------------------------------------------------------------------------------------------------------------------------
	pc.BeginTiming();
	uint8_t* bytes = framebuffer->GetDataPtr();
	for (uint32_t row = 0u; row < framebuffer->GetHeight(); row++)
	{
		for (uint32_t column = 0u; column < framebuffer->GetWidth(); column++)
		{
			const uint32_t rayIndex = (row * framebuffer->GetWidth() + column);

			// Byte offsets
			const uint32_t texelByteIndex = (row * framebuffer->GetWidth() * framebuffer->GetNumChannels()) + (column * framebuffer->GetNumChannels());
			assert(texelByteIndex < (framebuffer->GetWidth() * framebuffer->GetHeight() * framebuffer->GetNumChannels()));

			float red = 0.0f;
			float green = 0.0f;
			float blue = 0.0f;

			if ((cKeyState == 0u) && (vKeyState == 0u) && (bKeyState == 0u) && (nKeyState == 0u) && (mKeyState == 0u) && (xKeyState == 0u) && (lKeyState == 0u) && (kKeyState == 0u))
			{
				Vector3 radiance(0.0f, 0.0f, 0.0f);
				const uint32_t numSamples = 1u;
				const uint32_t depth = 1u;

				for (uint32_t sample = 0u; sample < numSamples; sample++)
				{
					//Vector3 texelTopLeft;
					//Vector3 texelBottomRight;
					//
					//texelTopLeft.SetX(m_texelCenters[rayIndex].X() - m_viewportDesc.m_texelWidth / 2.0f);
					//texelTopLeft.SetY(m_texelCenters[rayIndex].Y() + m_viewportDesc.m_texelHeight / 2.0f);
					//
					//texelBottomRight.SetX(m_texelCenters[rayIndex].X() + m_viewportDesc.m_texelWidth / 2.0f);
					//texelBottomRight.SetY(m_texelCenters[rayIndex].Y() - m_viewportDesc.m_texelHeight / 2.0f);
					//
					//const float randomX = RandomFloat(texelTopLeft.X(), texelBottomRight.X());
					//const float randomY = RandomFloat(texelBottomRight.Y(), texelTopLeft.Y());
					//
					//const Ray c_primaryRay(m_camera.GetCameraLocation(), Vector3(randomX, randomY, -1.0f));
					Ray c_primaryRay(m_camera.GetCameraLocation(), Vector3(m_texelCenters[rayIndex]));

					radiance = radiance + PathTrace(c_primaryRay, rayIndex, depth);
				}

				radiance = Vector3(radiance.X() / (float)numSamples, radiance.Y() / (float)numSamples, radiance.Z() / (float)numSamples);

				red = std::fmin(1.0f, radiance.X());
				green = std::fmin(1.0f, radiance.Y());
				blue = std::fmin(1.0f, radiance.Z());
			}

			if (xKeyState > 0u)
			{
				// Ray values reset here, so no need to do a reset on the vector
				Ray primaryRay(m_camera.GetCameraLocation(), m_texelCenters[rayIndex]);

#ifdef TRACE_AGAINST_BVH2
				const HitResult hr = TraceAgainstBVH2<false>(primaryRay, rayIndex, 1e-5f);
				primaryRayAABBIntersectionsCount[rayIndex] = (primaryRay.m_primaryAABBIntersectionTests);
				primaryRayTriangleIntersectionsCount[rayIndex] = (primaryRay.m_primaryTriangleIntersectionTests);
				primaryRayNodeVisits[rayIndex] = primaryRay.m_primaryNodeVisits;
#endif
#ifdef TRACE_AGAINST_BVH4
				const HitResult hr = TraceAgainstBVH4<false>(primaryRay, rayIndex, 1e-5f);
				primaryRayAABBIntersectionsCount[rayIndex] = (primaryRay.m_primaryAABBIntersectionTests);
				primaryRayTriangleIntersectionsCount[rayIndex] = (primaryRay.m_primaryTriangleIntersectionTests);
				primaryRayNodeVisits[rayIndex] = primaryRay.m_primaryNodeVisits;
#endif
#ifdef TRACE_AGAINST_NON_BVH
				const HitResult hr = TraceRayNonBVH<false>(primaryRay, rayIndex, 1e-5f);
				primaryRayTriangleIntersectionsCount[rayIndex] = (primaryRay.m_primaryTriangleIntersectionTests);
#endif
#ifdef TRACE_AGAINST_NON_BVH_SSE
				const HitResult hr = TraceRay4NonBVH<false>(primaryRay, rayIndex, 1e-5f);
				primaryRayTriangleIntersectionsCount[rayIndex] = (primaryRay.m_primaryTriangleIntersectionTests);
#endif

				if ((row == framebuffer->GetHeight() - 1u) && (column == framebuffer->GetWidth() - 1u))
				{
					// Average aabb intersections for primary rays
					uint32_t averageAABBVisits = 0u;
					for (uint32_t i = 0u; i < primaryRayAABBIntersectionsCount.size(); i++)
					{
						averageAABBVisits += primaryRayAABBIntersectionsCount[i];
					}

					averageAABBVisits = averageAABBVisits / (uint32_t)primaryRayAABBIntersectionsCount.size();

					char msgBuffer1[128u];
					sprintf_s(msgBuffer1, "Average AABB visits: %u \n", averageAABBVisits);
					OutputDebugStringA(msgBuffer1);


					// Average triangle intersections for primary rays
					uint32_t averageTriangleVisits = 0u;
					for (uint32_t i = 0u; i < primaryRayTriangleIntersectionsCount.size(); i++)
					{
						averageTriangleVisits += primaryRayTriangleIntersectionsCount[i];
					}

					averageTriangleVisits = averageTriangleVisits / (uint32_t)primaryRayTriangleIntersectionsCount.size();

					char msgBuffer2[128u];
					sprintf_s(msgBuffer2, "Average Triangle visits: %u \n", averageTriangleVisits);
					OutputDebugStringA(msgBuffer2);

					// Average node visits
					uint32_t averageNodeVisits = 0u;
					for (uint32_t i = 0u; i < primaryRayNodeVisits.size(); i++)
					{
						averageNodeVisits += primaryRayNodeVisits[i];
					}

					averageNodeVisits = averageNodeVisits / (uint32_t)primaryRayNodeVisits.size();

					char msgBuffer3[128u];
					sprintf_s(msgBuffer3, "Average Node visits: %u \n", averageNodeVisits);
					OutputDebugStringA(msgBuffer3);
				}

				red = 0.0f;
				green = 1.0f;
				blue = 0.0f;
			}

			if (cKeyState > 0u)
			{
				Ray primaryRay(m_camera.GetCameraLocation(), m_texelCenters[rayIndex]);

#ifdef TRACE_AGAINST_BVH2
				const HitResult hr = TraceAgainstBVH2<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH4
				const HitResult hr = TraceAgainstBVH4<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH
				const HitResult hr = TraceRayNonBVH<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH_SSE
				const HitResult hr = TraceRay4NonBVH<false>(primaryRay, rayIndex, 1e-5f);
#endif

				//const float clampValue = std::fmin(std::fmax(Dot(m_lightDirection, primaryHr.m_normal), 0.0f), 1.0f);

				if (hr.m_t != INFINITY)
				{
					Ray shadowRay(hr.m_intersectionPoint, m_lightDirection);

#ifdef TRACE_AGAINST_BVH2
					const HitResult shadowHr = TraceAgainstBVH2<true>(shadowRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH4
					const HitResult shadowHr = TraceAgainstBVH4<true>(shadowRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH
					const HitResult shadowHr = TraceRayNonBVH<true>(shadowRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH_SSE
					const HitResult shadowHr = TraceRay4NonBVH<true>(shadowRay, rayIndex, 1e-5f);
#endif

					red = (shadowHr.m_t == INFINITY) ? 1.0f : 0.0f;
					green = (shadowHr.m_t == INFINITY) ? 1.0f : 0.0f;
					blue = (shadowHr.m_t == INFINITY) ? 1.0f : 0.0f;
				}
				else
				{
					red = 0.7f;
					green = 0.7f;
					blue = 0.7f;
				}
			}

			if (vKeyState > 0u)
			{
				Ray primaryRay(m_camera.GetCameraLocation(), m_texelCenters[rayIndex]);

#ifdef TRACE_AGAINST_BVH2
				const HitResult hr = TraceAgainstBVH2<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH4
				const HitResult hr = TraceAgainstBVH4<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH
				const HitResult hr = TraceRayNonBVH<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH_SSE
				const HitResult hr = TraceRay4NonBVH<false>(primaryRay, rayIndex, 1e-5f);
#endif

				const float maxDepth = 10.0f;

				red = (hr.m_t != INFINITY) ? hr.m_intersectionPoint.X() / maxDepth : 0.0f;
				green = (hr.m_t != INFINITY) ? hr.m_intersectionPoint.Y() / maxDepth : 0.0f;
				blue = (hr.m_t != INFINITY) ? hr.m_intersectionPoint.Z() / maxDepth : 0.0f;
			}

			if (bKeyState > 0u)
			{
				Ray primaryRay(m_camera.GetCameraLocation(), m_texelCenters[rayIndex]);

#ifdef TRACE_AGAINST_BVH2
				const HitResult hr = TraceAgainstBVH2<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH4
				const HitResult hr = TraceAgainstBVH4<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH
				const HitResult hr = TraceRayNonBVH<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH_SSE
				const HitResult hr = TraceRay4NonBVH<false>(primaryRay, rayIndex, 1e-5f);
#endif

				const float maxDepth = 10.0f;

				red = (hr.m_t != INFINITY) ? hr.m_t / maxDepth : 0.0f;
				green = (hr.m_t != INFINITY) ? hr.m_t / maxDepth : 0.0f;
				blue = (hr.m_t != INFINITY) ? hr.m_t / maxDepth : 0.0f;
			}

			if (nKeyState > 0u)
			{
				Ray primaryRay(m_camera.GetCameraLocation(), m_texelCenters[rayIndex]);

#ifdef TRACE_AGAINST_BVH2
				const HitResult hr = TraceAgainstBVH2<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH4
				const HitResult hr = TraceAgainstBVH4<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH
				const HitResult hr = TraceRayNonBVH<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH_SSE
				const HitResult hr = TraceRay4NonBVH<false>(primaryRay, rayIndex, 1e-5f);
#endif

				red = (hr.m_t < INFINITY) ? 0.5f * hr.m_normal.X() + 0.5f : 0.0f;
				green = (hr.m_t < INFINITY) ? 0.5f * hr.m_normal.Y() + 0.5f : 0.0f;
				blue = (hr.m_t < INFINITY) ? 0.5f * hr.m_normal.Z() + 0.5f : 0.0f;
			}

			if (mKeyState > 0u)
			{
				Ray primaryRay(m_camera.GetCameraLocation(), m_texelCenters[rayIndex]);

#ifdef TRACE_AGAINST_BVH2
				const HitResult hr = TraceAgainstBVH2<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH4
				const HitResult hr = TraceAgainstBVH4<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH
				const HitResult hr = TraceRayNonBVH<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH_SSE
				const HitResult hr = TraceRay4NonBVH<false>(primaryRay, rayIndex, 1e-5f);
#endif

				red = (hr.m_primitiveId != UINT32_MAX) ? primitiveDebugColours[hr.m_primitiveId % 5u].X() : 0.0f;
				green = (hr.m_primitiveId != UINT32_MAX) ? primitiveDebugColours[hr.m_primitiveId % 5u].Y() : 0.0f;
				blue = (hr.m_primitiveId != UINT32_MAX) ? primitiveDebugColours[hr.m_primitiveId % 5u].Z() : 0.0f;
			}

			if (lKeyState > 0u)
			{
				Ray primaryRay(m_camera.GetCameraLocation(), m_texelCenters[rayIndex]);

#ifdef TRACE_AGAINST_BVH2
				const HitResult hr = TraceAgainstBVH2<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH4
				const HitResult hr = TraceAgainstBVH4<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH
				const HitResult hr = TraceRayNonBVH<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH_SSE
				const HitResult hr = TraceRay4NonBVH<false>(primaryRay, rayIndex, 1e-5f);
#endif

				red = (hr.m_materialId != UINT32_MAX) ? m_sceneManager->GetDebugMaterialColour(hr.m_materialId).X() : 0.0f;
				green = (hr.m_materialId != UINT32_MAX) ? m_sceneManager->GetDebugMaterialColour(hr.m_materialId).Y() : 0.0f;
				blue = (hr.m_materialId != UINT32_MAX) ? m_sceneManager->GetDebugMaterialColour(hr.m_materialId).Z() : 0.0f;
			}

			// Texture coordinates
			if (kKeyState > 0u)
			{
				Ray primaryRay(m_camera.GetCameraLocation(), m_texelCenters[rayIndex]);

#ifdef TRACE_AGAINST_BVH2
				const HitResult hr = TraceAgainstBVH2<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH4
				const HitResult hr = TraceAgainstBVH4<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH
				const HitResult hr = TraceRayNonBVH<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH_SSE
				const HitResult hr = TraceRay4NonBVH<false>(primaryRay, rayIndex, 1e-5f);
#endif

				red = (hr.m_primitiveId != UINT32_MAX) ? hr.m_texCoords.X() : 0.0f;
				green = (hr.m_primitiveId != UINT32_MAX) ? hr.m_texCoords.Y() : 0.0f;
				blue = 0.0f;
			}

			bytes[texelByteIndex] = uint8_t(red * 255.0f);
			bytes[texelByteIndex + 1u] = uint8_t(green * 255.0f);
			bytes[texelByteIndex + 2u] = uint8_t(blue * 255.0f);
			bytes[texelByteIndex + 3u] = 1u;
		}
	}

	pc.EndTiming();

	char buffer[128];
	sprintf_s(buffer, "Total frame time: %f \n", pc.GetMilliseconds());
	OutputDebugStringA(buffer);
}

// --------------------------------------------------------------------------------
void Renderer::RegenerateViewSpaceDirections(Framebuffer* framebuffer)
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

	for (uint32_t row = 0u; row < framebuffer->GetHeight(); row++)
	{
		const float ty = (row * m_viewportDesc.m_texelHeight) / m_viewportDesc.m_height;
		const Vector3 c_r0 = (1.0f - ty) * m_viewportDesc.m_topLeftTexel + ty * m_viewportDesc.m_bottomLeftTexel;
		const Vector3 c_r1 = (1.0f - ty) * m_viewportDesc.m_topRightTexel + ty * m_viewportDesc.m_bottomRightTexel;

		for (uint32_t column = 0u; column < framebuffer->GetWidth(); column++)
		{
			const float tx = (column * m_viewportDesc.m_texelWidth) / m_viewportDesc.m_width;

			Vector3 texelCenter = (1.0f - tx) * c_r0 + tx * c_r1;
			const float xVal = texelCenter.X() + (m_viewportDesc.m_texelWidth / 2.0f);

			const float valDegreesToRad = 90.0f * ((float)M_PI / 180.0f);
			const float rotatedX = cosf(valDegreesToRad) * xVal + sinf(valDegreesToRad) * texelCenter.Z();
			const float rotatedZ = -sinf(valDegreesToRad) * xVal + cosf(valDegreesToRad) * texelCenter.Z();

			//texelCenter.SetX(texelCenter.X() + (m_viewportDesc.m_texelWidth / 2.0f));
			texelCenter.SetX(rotatedX);
			texelCenter.SetY(texelCenter.Y() - (m_viewportDesc.m_texelHeight / 2.0f));
			texelCenter.SetZ(rotatedZ);
			m_texelCenters.push_back(texelCenter);
		}
	}

	m_isFirstFrame = false;
}

// --------------------------------------------------------------------------------
Vector3 Renderer::PathTrace(Ray& ray, const uint32_t rayIndex, uint32_t depth)
{
	if (depth <= 0u)
	{
		return Vector3(0.0f, 0.0f, 0.0f);
	}

	Vector3 radiance(0.0f, 0.0f, 0.0f);

#ifdef TRACE_AGAINST_BVH2
	const HitResult c_primaryHitResult = TraceAgainstBVH2<false>(ray, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH4
	const HitResult c_primaryHitResult = TraceAgainstBVH4<false>(ray, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH
	const HitResult c_primaryHitResult = TraceRayNonBVH<false>(ray, rayIndex, 1e-5f);
#endif 
#ifdef TRACE_AGAINST_NON_BVH_SSE
	const HitResult c_primaryHitResult = TraceRay4NonBVH<false>(ray, rayIndex, 1e-5f);
#endif 

	if (c_primaryHitResult.m_t != INFINITY)
	{
		// Indirect lighting
		{
			// Calculate the random direction of the outward ray
			Ray rayOnHemisphere(c_primaryHitResult.m_intersectionPoint, Vector3::RandomVector3OnHemisphere(c_primaryHitResult.m_normal));

			// RENDERING EQUATION

			// We need the Li
			const Vector3 Li = PathTrace(rayOnHemisphere, rayIndex, depth - 1u);

			// Elongation/cosine term, the falloff (Geometric term)
			// We use the ray direction, instead of -ray.Direction() so that the Dot product produces a positive value
			const float cosineTerm = std::fmin(std::fmax(Dot(rayOnHemisphere.Direction(), c_primaryHitResult.m_normal), 0.0f), 1.0f);

			// BRDF, in our case just use Lambert which is P/PI, P being the colour of the material, a vector3 [0,1] for each wavelength
			const Vector3 brdf = (1.0f / (float)M_PI) * c_primaryHitResult.m_colour;

			radiance = cosineTerm * brdf * Li;

			// Divide everything by the probability distribution function, for our case just 1/Pi
			const float pdf = 1.0f / (2.0f * (float)M_PI);
			radiance = Vector3(radiance.X() / pdf, radiance.Y() / pdf, radiance.Z() / pdf);
		}

		//Direct Lighting
		{
			const float clampValue = std::fmin(std::fmax(Dot(m_lightDirection, c_primaryHitResult.m_normal), 0.0f), 1.0f);

			Ray c_shadowRay(c_primaryHitResult.m_intersectionPoint, m_lightDirection);
#ifdef TRACE_AGAINST_BVH2
			const HitResult c_secondaryRayHitResult = TraceAgainstBVH2<true>(c_shadowRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH4
			const HitResult c_secondaryRayHitResult = TraceAgainstBVH4<true>(c_shadowRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH
			const HitResult c_secondaryRayHitResult = TraceRayNonBVH<true>(c_shadowRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH_SSE
			const HitResult c_secondaryRayHitResult = TraceRay4NonBVH<true>(c_shadowRay, rayIndex, 1e-5f);
#endif

			if (c_secondaryRayHitResult.m_t == INFINITY)
			{
				Vector3 directRadiance;
				directRadiance.SetX(clampValue * c_primaryHitResult.m_colour.X());
				directRadiance.SetY(clampValue * c_primaryHitResult.m_colour.Y());
				directRadiance.SetZ(clampValue * c_primaryHitResult.m_colour.Z());

				radiance = radiance + directRadiance;
			}
		}
	}
	else
	{
		const float val = 0.5f * (ray.Direction().Y() + 1.0f);
		const Vector3 skyColour = (1.0f - val) * Vector3(1.0f, 1.0f, 1.0f) + val * Vector3(0.5f, 0.7f, 1.0f);

		radiance.SetX(skyColour.X());
		radiance.SetY(skyColour.Y());
		radiance.SetZ(skyColour.Z());
	}

	// Use hit result to spawn other rays
	return radiance;
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
HitResult Renderer::TraceRayNonBVH(Ray& ray, const uint32_t rayIndex, const float tMin)
{
	// Probably should be moved elsewhere
#ifdef _DEBUG
	assert(rayIndex >= 0u);
#endif
#ifdef NDEBUG
	(void)(rayIndex);
#endif

	HitResult hitResult;
	bool hasHit = false;
	float tMax = INFINITY;
	float tu = FLT_MAX;
	float tv = FLT_MAX;
	uint32_t primitiveId = UINT32_MAX;

	const std::vector<TraversalTriangle>& traversalTriangles = m_traversalDataManager->GetTraversalTriangles();
	const TraversalTriangle* const beginTriangle = &traversalTriangles[0u];
	const TraversalTriangle* const endTriangle = beginTriangle + traversalTriangles.size();

	const std::vector<uint32_t>& materialIndices = m_traversalDataManager->GetMaterialIndices();
	const std::vector<TriangleTexCoords>& triangleTexCoords = m_traversalDataManager->GetTriangleTexCoords();

	uint32_t triangleIndex = 0u;
	for (const TraversalTriangle* triangle = beginTriangle; triangle != endTriangle; triangle++)
	{
		HitTriangle(ray, *triangle, triangleIndex, tMin, primitiveId, tMax, tu, tv, hasHit);

		if (T_acceptAnyHit)
		{
			if (hasHit)
			{
				break;
			}
		}

		triangleIndex++;
	}

	if (hasHit)
	{
		hitResult.m_t = tMax;

		hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(tMax);

		const TriangleTexCoords& texCoords = triangleTexCoords[primitiveId];
		hitResult.m_texCoords.SetX((1.0f - tu - tv) * texCoords.m_v0uv[0u] + tu * texCoords.m_v1uv[0u] + tv * texCoords.m_v2uv[0u]);
		hitResult.m_texCoords.SetY((1.0f - tu - tv) * texCoords.m_v0uv[1u] + tu * texCoords.m_v1uv[1u] + tv * texCoords.m_v2uv[1u]);

		hitResult.m_colour = Vector3(1.0f, 0.55f, 0.0f);

		const Vector3 edge1(traversalTriangles[primitiveId].m_edge1[0u], traversalTriangles[primitiveId].m_edge1[1u], traversalTriangles[primitiveId].m_edge1[2u]);
		const Vector3 edge2(traversalTriangles[primitiveId].m_edge2[0u], traversalTriangles[primitiveId].m_edge2[1u], traversalTriangles[primitiveId].m_edge2[2u]);
		const Vector3 normal = Normalize(Cross(edge1, edge2));
		hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;

		hitResult.m_primitiveId = primitiveId;

		hitResult.m_materialId = materialIndices[primitiveId];
	}

	return hitResult;
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
HitResult Renderer::TraceRay4NonBVH(Ray& ray, const uint32_t rayIndex, const float tMin)
{
	(void)rayIndex;
	HitResult hitResult;

	// Constants
	const __m128 epsilon = _mm_set1_ps(1e-8f);
	const __m128 zeros = _mm_set1_ps(0.0f);
	const __m128 ones = _mm_set1_ps(1.0f);

	// Ray data
	const __m128 rayOriginX = _mm_set1_ps(ray.Origin().X());
	const __m128 rayOriginY = _mm_set1_ps(ray.Origin().Y());
	const __m128 rayOriginZ = _mm_set1_ps(ray.Origin().Z());

	const __m128 rayDirectionX = _mm_set1_ps(ray.Direction().X());
	const __m128 rayDirectionY = _mm_set1_ps(ray.Direction().Y());
	const __m128 rayDirectionZ = _mm_set1_ps(ray.Direction().Z());

	// tMin
	const __m128 tMinimum = _mm_set1_ps(tMin);

	// Loop outputs
	__m128i outTri4Indices = _mm_set_epi32(INT_MAX, INT_MAX, INT_MAX, INT_MAX);
	__m128 outTMax = _mm_set1_ps(INFINITY);
	__m128 outU = _mm_set1_ps(FLT_MAX);
	__m128 outV = _mm_set1_ps(FLT_MAX);

	const std::vector<TraversalTriangle4>& traversalTriangle4s = m_traversalDataManager->GetTraversalTriangle4s();
	const TraversalTriangle4* const beginTriangle4 = &traversalTriangle4s[0u];
	const TraversalTriangle4* const endTriangle4 = beginTriangle4 + traversalTriangle4s.size();

	int currentIndex = 0u;
	int moveMask = 0u;
	for (const TraversalTriangle4* triangle4 = beginTriangle4; triangle4 != endTriangle4; triangle4++)
	{
		HitTriangle4(ray, *triangle4, currentIndex, tMin, outTri4Indices, outTMax, outU, outV, moveMask);

		if (T_acceptAnyHit) // This runs all the time for the sse version, the T_AcceptAnyHit runs only if the hit triangle returns something for the non-sse
		{
			if (moveMask)
			{
				break;
			}
		}

		currentIndex++;
	}

	// Get the closest t value out of the four
	const __m128 tShuffle23to01 = _mm_shuffle_ps(outTMax, outTMax, _MM_SHUFFLE(0, 0, 2, 3));
	const __m128 closestTwoTs = _mm_min_ps(outTMax, tShuffle23to01);
	const __m128i firstMinMask = _mm_castps_si128(_mm_cmplt_ps(outTMax, tShuffle23to01));

	const __m128 tShuffle1to0 = _mm_shuffle_ps(closestTwoTs, closestTwoTs, _MM_SHUFFLE(0, 0, 0, 1));
	const __m128 closestT = _mm_min_ps(closestTwoTs, tShuffle1to0);
	const __m128i closestMask = _mm_castps_si128(_mm_cmplt_ps(closestTwoTs, tShuffle1to0));

	// Shuffle to obtain the sub index of the closest t, within the original arrays
	const __m128i orderedIndices = _mm_set_epi32(3, 2, 1, 0);
	const __m128i indexShuffle23to01 = _mm_shuffle_epi32(orderedIndices, _MM_SHUFFLE(0, 0, 2, 3));
	const __m128i closestTwoIndices = _mm_or_epi32(_mm_and_epi32(firstMinMask, orderedIndices),
		_mm_andnot_epi32(firstMinMask, indexShuffle23to01));

	const __m128i indexShuffle1to0 = _mm_shuffle_epi32(closestTwoIndices, _MM_SHUFFLE(0, 0, 0, 1));
	const __m128i closestSubIndex = _mm_or_epi32(_mm_and_epi32(closestMask, closestTwoIndices),
		_mm_andnot_epi32(closestMask, indexShuffle1to0));

	const int subIndex = _mm_cvtsi128_si32(closestSubIndex);

	int tri4Indices[4u];
	_mm_storeu_epi32(tri4Indices, outTri4Indices);
	int tri4Index = tri4Indices[subIndex];
	if (tri4Index != INT_MAX)
	{
		float tMaxes[4u];
		_mm_storeu_ps(tMaxes, outTMax);
		float tMax = tMaxes[subIndex];

		float us[4u];
		_mm_storeu_ps(us, outU);
		float u = us[subIndex];

		float vs[4u];
		_mm_storeu_ps(vs, outV);
		float v = vs[subIndex];

		hitResult.m_t = tMax;

		hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(tMax);

		const std::vector<TriangleTexCoords4>& texCoords4 = m_traversalDataManager->GetTriangleTexCoords4();
		hitResult.m_texCoords.SetX((1.0f - u - v) * texCoords4[tri4Index].m_v0U[subIndex] + u * texCoords4[tri4Index].m_v1U[subIndex] + v * texCoords4[tri4Index].m_v2U[subIndex]);
		hitResult.m_texCoords.SetY((1.0f - u - v) * texCoords4[tri4Index].m_v0V[subIndex] + u * texCoords4[tri4Index].m_v1V[subIndex] + v * texCoords4[tri4Index].m_v2V[subIndex]);

		hitResult.m_colour = Vector3(1.0f, 0.55f, 0.0f);

		const Vector3 edge1(traversalTriangle4s[tri4Index].m_edge1X[subIndex], traversalTriangle4s[tri4Index].m_edge1Y[subIndex], traversalTriangle4s[tri4Index].m_edge1Z[subIndex]);
		const Vector3 edge2(traversalTriangle4s[tri4Index].m_edge2X[subIndex], traversalTriangle4s[tri4Index].m_edge2Y[subIndex], traversalTriangle4s[tri4Index].m_edge2Z[subIndex]);
		const Vector3 normal = Normalize(Cross(edge1, edge2));
		hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;

		// Try and get the indices for this too
		hitResult.m_primitiveId = 0u;

		hitResult.m_materialId = 0u;
	}

	return hitResult;
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
HitResult Renderer::TraceAgainstBVH2(Ray& ray, const uint32_t rayIndex, const float tMin)
{
	(void)rayIndex;

	HitResult hitResult;
	bool hasHit = false;
	float tMax = INFINITY;
	float tu = FLT_MAX;
	float tv = FLT_MAX;
	uint32_t primitiveId = UINT32_MAX;

	BVH2DFSTraversal<T_acceptAnyHit>(0u, ray, tMin, primitiveId, tMax, tu, tv, hasHit);
	
	if (hasHit)
	{
		hitResult.m_t = tMax;

		hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(tMax);


		const TriangleTexCoords& texCoords = m_traversalDataManager->GetBVH2TriangleTexCoords(primitiveId);
		hitResult.m_texCoords.SetX((1.0f - tu - tv) * texCoords.m_v0uv[0u] + tu * texCoords.m_v1uv[0u] + tv * texCoords.m_v2uv[0u]);
		hitResult.m_texCoords.SetY((1.0f - tu - tv) * texCoords.m_v0uv[1u] + tu * texCoords.m_v1uv[1u] + tv * texCoords.m_v2uv[1u]);

		hitResult.m_colour = Vector3(1.0f, 0.55f, 0.0f);

		const TraversalTriangle& traversalTriangle = m_traversalDataManager->GetBVH2TraversalTriangle(primitiveId);
		const Vector3 edge1(traversalTriangle.m_edge1[0u], traversalTriangle.m_edge1[1u], traversalTriangle.m_edge1[2u]);
		const Vector3 edge2(traversalTriangle.m_edge2[0u], traversalTriangle.m_edge2[1u], traversalTriangle.m_edge2[2u]);
		const Vector3 normal = Normalize(Cross(edge1, edge2));
		hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;

		hitResult.m_primitiveId = primitiveId;

		hitResult.m_materialId = m_traversalDataManager->GetBVH2MaterialIndex(primitiveId);
	}

	return hitResult;
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
void Renderer::BVH2DFSTraversal(const uint32_t innerNodeStartIndex, Ray& ray, const float tMin, uint32_t& out_primitiveId,
	float& out_tMax, float& out_u, float& out_v, bool& out_hasHit)
{
	if (!T_acceptAnyHit)
	{
		ray.m_primaryNodeVisits++;
	}

	const BVH2InnerNode& node = m_traversalDataManager->GetBVH2InnerNode(innerNodeStartIndex);

	float tNears[2u] = { INFINITY, INFINITY };
	float hit[2u] = { false, false };
	hit[0u] = RayAABBIntersection(ray, T_acceptAnyHit, node.m_leftAABB.m_min.X(), node.m_leftAABB.m_min.Y(), node.m_leftAABB.m_min.Z(),
		node.m_leftAABB.m_max.X(), node.m_leftAABB.m_max.Y(), node.m_leftAABB.m_max.Z(), out_tMax, &tNears[0u]);
	hit[1u] = RayAABBIntersection(ray, T_acceptAnyHit, node.m_rightAABB.m_min.X(), node.m_rightAABB.m_min.Y(), node.m_rightAABB.m_min.Z(),
		node.m_rightAABB.m_max.X(), node.m_rightAABB.m_max.Y(), node.m_rightAABB.m_max.Z(), out_tMax, &tNears[0u]);

	if (!hit[0u] && !hit[1u])
	{
		return;
	}

	// Reorder if needed
	uint32_t visitOrder[2u] = { node.m_leftChild, node.m_rightChild };
	AABB aabbs[2u] = { node.m_leftAABB, node.m_rightAABB };
	if (tNears[0u] > tNears[1u])
	{
		std::swap(visitOrder[0u], visitOrder[1u]);
		std::swap(hit[0u], hit[1u]);

		const AABB tempAABB = aabbs[0u];
		aabbs[0u] = aabbs[1u];
		aabbs[1u] = tempAABB;
	}

	// Traversal
	for (uint32_t child = 0u; child < 2u; child++)
	{
		if (!hit[child])
		{
			continue;
		}

		if (visitOrder[child] >> 31u)
		{
			const uint32_t triangleIndex = visitOrder[child] & ~(1u << 31u);
			const TraversalTriangle& traversalTriangle = m_traversalDataManager->GetBVH2TraversalTriangle(triangleIndex);
			HitTriangle(ray, traversalTriangle, triangleIndex, tMin, out_primitiveId, out_tMax, out_u, out_v, out_hasHit);
		}
		else if (RayAABBIntersection(ray, T_acceptAnyHit, aabbs[child].m_min.X(), aabbs[child].m_min.Y(), aabbs[child].m_min.Z(),
			aabbs[child].m_max.X(), aabbs[child].m_max.Y(), aabbs[child].m_max.Z(), out_tMax, nullptr))
		{
			BVH2DFSTraversal<T_acceptAnyHit>(visitOrder[child], ray, tMin, out_primitiveId, out_tMax, out_u, out_v, out_hasHit);
		}

		if constexpr (T_acceptAnyHit)
		{
			if (out_hasHit)
			{
				return;
			}
		}
	}
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
HitResult Renderer::TraceAgainstBVH4(Ray& ray, const uint32_t rayIndex, const float tMin)
{
	(void)rayIndex;

	HitResult hitResult;

	__m128i outTri4Indices = _mm_set_epi32(INT_MAX, INT_MAX, INT_MAX, INT_MAX);
	__m128 outTMax = _mm_set1_ps(INFINITY);
	__m128 outU = _mm_set1_ps(FLT_MAX);
	__m128 outV = _mm_set1_ps(FLT_MAX);
	int moveMask = 0;

	BVH4DFSTraversal<T_acceptAnyHit>(0u, ray, tMin, outTri4Indices, outTMax, outU, outV, moveMask);

	// Get the closest t value out of the four
	const __m128 tShuffle23to01 = _mm_shuffle_ps(outTMax, outTMax, _MM_SHUFFLE(0, 0, 2, 3));
	const __m128 closestTwoTs = _mm_min_ps(outTMax, tShuffle23to01);
	const __m128i firstMinMask = _mm_castps_si128(_mm_cmplt_ps(outTMax, tShuffle23to01));

	const __m128 tShuffle1to0 = _mm_shuffle_ps(closestTwoTs, closestTwoTs, _MM_SHUFFLE(0, 0, 0, 1));
	const __m128 closestT = _mm_min_ps(closestTwoTs, tShuffle1to0);
	const __m128i closestMask = _mm_castps_si128(_mm_cmplt_ps(closestTwoTs, tShuffle1to0));

	// Shuffle to obtain the sub index of the closest t, within the original arrays
	const __m128i orderedIndices = _mm_set_epi32(3, 2, 1, 0);
	const __m128i indexShuffle23to01 = _mm_shuffle_epi32(orderedIndices, _MM_SHUFFLE(0, 0, 2, 3));
	const __m128i closestTwoIndices = _mm_or_epi32(_mm_and_epi32(firstMinMask, orderedIndices),
		_mm_andnot_epi32(firstMinMask, indexShuffle23to01));

	const __m128i indexShuffle1to0 = _mm_shuffle_epi32(closestTwoIndices, _MM_SHUFFLE(0, 0, 0, 1));
	const __m128i closestSubIndex = _mm_or_epi32(_mm_and_epi32(closestMask, closestTwoIndices),
		_mm_andnot_epi32(closestMask, indexShuffle1to0));

	const int subIndex = _mm_cvtsi128_si32(closestSubIndex);

	int tri4Indices[4u];
	_mm_storeu_epi32(tri4Indices, outTri4Indices);
	int tri4Index = tri4Indices[subIndex];
	if (tri4Index != INT_MAX)
	{
		float tMaxes[4u];
		_mm_storeu_ps(tMaxes, outTMax);
		float tMax = tMaxes[subIndex];

		float us[4u];
		_mm_storeu_ps(us, outU);
		float u = us[subIndex];

		float vs[4u];
		_mm_storeu_ps(vs, outV);
		float v = vs[subIndex];

		hitResult.m_t = tMax;

		hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(tMax);

		const TriangleTexCoords4& triangleTexCoords4 = m_traversalDataManager->GetBVH4TriangleTexCoords4(tri4Index);
		hitResult.m_texCoords.SetX((1.0f - u - v) * triangleTexCoords4.m_v0U[subIndex] + u * triangleTexCoords4.m_v1U[subIndex] + v * triangleTexCoords4.m_v2U[subIndex]);
		hitResult.m_texCoords.SetY((1.0f - u - v) * triangleTexCoords4.m_v0V[subIndex] + u * triangleTexCoords4.m_v1V[subIndex] + v * triangleTexCoords4.m_v2V[subIndex]);

		hitResult.m_colour = Vector3(1.0f, 0.55f, 0.0f);

		const TraversalTriangle4& traversalTriangle4 = m_traversalDataManager->GetBVH4TraversalTriangle4(tri4Index);
		const Vector3 edge1(traversalTriangle4.m_edge1X[subIndex], traversalTriangle4.m_edge1Y[subIndex], traversalTriangle4.m_edge1Z[subIndex]);
		const Vector3 edge2(traversalTriangle4.m_edge2X[subIndex], traversalTriangle4.m_edge2Y[subIndex], traversalTriangle4.m_edge2Z[subIndex]);
		const Vector3 normal = Normalize(Cross(edge1, edge2));
		hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;

		const TriangleIndices& triangleIndices = m_traversalDataManager->GetBVH4TriangleIndices(tri4Index);
		hitResult.m_primitiveId = triangleIndices.m_triangleIndices[subIndex];

		const Material4Index& material4Index = m_traversalDataManager->GetBVH4Material4Index(tri4Index);
		hitResult.m_materialId = material4Index.m_indices[subIndex];
	}

	return hitResult;
}

//#define SORTED_BVH4
// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
void Renderer::BVH4DFSTraversal(const uint32_t innerNodeStartIndex, Ray& ray, const float tMin, __m128i& out_primitiveId, __m128& out_tMax, __m128& out_u, __m128& out_v, int& moveMask)
{
	if (!T_acceptAnyHit)
	{
		ray.m_primaryNodeVisits++;
		ray.m_primaryAABBIntersectionTests += 4u;
	}

	const BVH4InnerNode& node = m_traversalDataManager->GetBVH4InnerNode(innerNodeStartIndex);

#ifdef SORTED_BVH4

#if _DEBUG
	float theTNears[4u] = { INFINITY, INFINITY, INFINITY, INFINITY };
	int32_t theTNearsAsInts[4u] = { INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX };
	int32_t theTNearsAfterMask[4u] = { INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX };
	int32_t theTNearsAfterShiftLeft[4u] = { INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX };
	int32_t theCombinedTNearAndChildIndex[4u] = { INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX };
#endif

	uint32_t hitCount = 0u;
	int32_t nearPlaneAndChildIndex[4u] = { INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX };
	for (uint32_t child = 0u; child < 4u; child++)
	{
		theTNearsAfterMask[child] = theTNearsAsInts[child] & 0x3FFFFFFE;
		theTNearsAfterShiftLeft[child] = theTNearsAfterMask[child] << 1;
		theCombinedTNearAndChildIndex[child] = theTNearsAfterShiftLeft[child] | child;

		float tNear = INFINITY;
		if (RayAABBIntersection(ray, T_acceptAnyHit, node.m_aabbMinX[child], node.m_aabbMinY[child], node.m_aabbMinZ[child], node.m_aabbMaxX[child],
			node.m_aabbMaxY[child], node.m_aabbMaxZ[child], tMax, &tNear))
		{
#if _DEBUG
			theTNears[child] = tNear;
			theTNearsAsInts[child] = *((int32_t*)&tNear);
			theTNearsAfterMask[child] = theTNearsAsInts[child] & 0x3FFFFFFE;
			theTNearsAfterShiftLeft[child] = theTNearsAfterMask[child] << 1;
			theCombinedTNearAndChildIndex[child] = theTNearsAfterShiftLeft[child] | child;
#endif
			nearPlaneAndChildIndex[child] = ((*(int32_t*)&tNear & 0x3FFFFFFE) << 1) | child;
			hitCount++;
		}
	}

	// Early out if no hit
	if (!hitCount)
	{
		return;
	}

	// Sort in ascending order
#if 1
	const int32_t a = std::min(nearPlaneAndChildIndex[0u], nearPlaneAndChildIndex[1u]);
	const int32_t b = std::max(nearPlaneAndChildIndex[0u], nearPlaneAndChildIndex[1u]);
	const int32_t c = std::min(nearPlaneAndChildIndex[2u], nearPlaneAndChildIndex[3u]);
	const int32_t d = std::max(nearPlaneAndChildIndex[2u], nearPlaneAndChildIndex[3u]);

	int32_t visitOrder[4u] = { INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX };
	visitOrder[0u] = std::min(a, c);
	visitOrder[2u] = std::max(a, c);
	visitOrder[1u] = std::min(b, d);
	visitOrder[3u] = std::max(b, d);

	const int32_t e = std::min(visitOrder[1u], visitOrder[2u]);
	const int32_t f = std::max(visitOrder[1u], visitOrder[2u]);

	visitOrder[1u] = e;
	visitOrder[2u] = f;

#else
	if (nearPlaneAndChildIndex[0u] > nearPlaneAndChildIndex[1u]) { std::swap(nearPlaneAndChildIndex[0u], nearPlaneAndChildIndex[1u]); }
	if (nearPlaneAndChildIndex[2u] > nearPlaneAndChildIndex[3u]) { std::swap(nearPlaneAndChildIndex[2u], nearPlaneAndChildIndex[3u]); }
	if (nearPlaneAndChildIndex[0u] > nearPlaneAndChildIndex[2u]) { std::swap(nearPlaneAndChildIndex[0u], nearPlaneAndChildIndex[2u]); }
	if (nearPlaneAndChildIndex[1u] > nearPlaneAndChildIndex[3u]) { std::swap(nearPlaneAndChildIndex[1u], nearPlaneAndChildIndex[3u]); }
	if (nearPlaneAndChildIndex[1u] > nearPlaneAndChildIndex[2u]) { std::swap(nearPlaneAndChildIndex[1u], nearPlaneAndChildIndex[2u]); }
#endif

#ifdef _DEBUG
	//for (uint32_t visitNode = 0u; visitNode < 3u; visitNode++)
	//{
	//	assert(nearPlaneAndChildIndex[visitNode] <= nearPlaneAndChildIndex[visitNode + 1u]);
	//}
#endif

	//Traversal
	for (uint32_t child = 0u; child < hitCount; child++)
	{
#if 1
		const uint32_t visitIndex = (uint32_t)(visitOrder[child] & 0x00000003);
#else
		const uint32_t visitIndex = (uint32_t)nearPlaneAndChildIndex[child] & 0x00000003;
#endif
		if (node.m_child[visitIndex] >> 31u)
		{
			const uint32_t triangleIndex = node.m_child[visitIndex] & ~(1u << 31u);
			HitTriangle<T_acceptAnyHit>(ray, rayIndex, tMin, tMax, triangleIndex, out_hitResult, out_hasHit);
		}
		else
		{
			BVH4DFSTraversal<T_acceptAnyHit>(node.m_child[visitIndex], ray, rayIndex, tMin, tMax, out_hitResult, out_hasHit);
		}

		if constexpr (T_acceptAnyHit)
		{
			if (out_hasHit)
			{
				return;
			}
		}
	}
#endif

	// 0 and tMAx
	const __m128 zeroReg = _mm_set1_ps(0.0f);

	// AABB paramaters
	const __m128 minXs = _mm_loadu_ps(node.m_aabbMinX);
	const __m128 minYs = _mm_loadu_ps(node.m_aabbMinY);
	const __m128 minZs = _mm_loadu_ps(node.m_aabbMinZ);
	const __m128 maxXs = _mm_loadu_ps(node.m_aabbMaxX);
	const __m128 maxYs = _mm_loadu_ps(node.m_aabbMaxY);
	const __m128 maxZs = _mm_loadu_ps(node.m_aabbMaxZ);

	//Ray data
	const __m128 rayInverseX = _mm_set1_ps(ray.InverseDirection().X());
	const __m128 rayInverseY = _mm_set1_ps(ray.InverseDirection().Y());
	const __m128 rayInverseZ = _mm_set1_ps(ray.InverseDirection().Z());

	const __m128 rayNegativeOriginTimesInvDirX = _mm_set1_ps(ray.NegativeOriginTimesInvDir().X());
	const __m128 rayNegativeOriginTimesInvDirY = _mm_set1_ps(ray.NegativeOriginTimesInvDir().Y());
	const __m128 rayNegativeOriginTimesInvDirZ = _mm_set1_ps(ray.NegativeOriginTimesInvDir().Z());

	// TNears
	const __m128 t0X = _mm_fmadd_ps(minXs, rayInverseX, rayNegativeOriginTimesInvDirX);
	const __m128 t0Y = _mm_fmadd_ps(minYs, rayInverseY, rayNegativeOriginTimesInvDirY);
	const __m128 t0Z = _mm_fmadd_ps(minZs, rayInverseZ, rayNegativeOriginTimesInvDirZ);

	// TFars
	const __m128 t1X = _mm_fmadd_ps(maxXs, rayInverseX, rayNegativeOriginTimesInvDirX);
	const __m128 t1Y = _mm_fmadd_ps(maxYs, rayInverseY, rayNegativeOriginTimesInvDirY);
	const __m128 t1Z = _mm_fmadd_ps(maxZs, rayInverseZ, rayNegativeOriginTimesInvDirZ);

	// Entries and exits
	const __m128 enterX = _mm_min_ps(t0X, t1X);
	const __m128 enterY = _mm_min_ps(t0Y, t1Y);
	const __m128 enterZ = _mm_min_ps(t0Z, t1Z);

	const __m128 exitX = _mm_max_ps(t0X, t1X);
	const __m128 exitY = _mm_max_ps(t0Y, t1Y);
	const __m128 exitZ = _mm_max_ps(t0Z, t1Z);

	// t0 and t1
	const __m128 t0 = _mm_max_ps(_mm_max_ps(enterX, enterY), _mm_max_ps(zeroReg, enterZ));
	const __m128 t1 = _mm_min_ps(_mm_min_ps(exitX, exitY), _mm_min_ps(out_tMax, exitZ));

	// hasIntersected
	const __m128 hasIntersected = _mm_cmpge_ps(t1, t0);
	const int intersectionMask = _mm_movemask_ps(hasIntersected);

	if (!intersectionMask)
	{
		return;
	}

	// Packing
	const __m128i int32Max = _mm_set1_epi32(INT32_MAX);
	const __m128i t0AsInts = _mm_castps_si128(_mm_or_ps(_mm_and_ps(hasIntersected, t0), _mm_andnot_ps(hasIntersected, _mm_castsi128_ps(int32Max))));
	const __m128i postChopBits = _mm_and_si128(t0AsInts, _mm_set1_epi32(0x3FFFFFFE));
	const __m128i shiftedLeft = _mm_slli_epi32(postChopBits, 1);

	const __m128i childIndicesInOrder = _mm_set_epi32(3, 2, 1, 0);
	const __m128i combinedT0AndIndex = _mm_or_epi32(shiftedLeft, childIndicesInOrder);

	// Sort
	const __m128i shuffle0 = _mm_shuffle_epi32(combinedT0AndIndex, _MM_SHUFFLE(1, 0, 1, 0));
	const __m128i min0 = _mm_min_epi32(combinedT0AndIndex, shuffle0);
	const __m128i max0 = _mm_max_epi32(combinedT0AndIndex, shuffle0);

	const __m128i shuffle1 = _mm_shuffle_epi32(max0, _MM_SHUFFLE(2, 3, 1, 0));;
	const __m128i min1 = _mm_min_epi32(min0, shuffle1);
	const __m128i max1 = _mm_max_epi32(min0, shuffle1);

	const __m128i shuffle2 = _mm_shuffle_epi32(max1, _MM_SHUFFLE(2, 3, 1, 0));
	const __m128i l3 = _mm_max_epi32(max1, shuffle2);
	const __m128i l2 = _mm_min_epi32(max1, shuffle2);

	const __m128i shuffle3 = _mm_shuffle_epi32(min1, _MM_SHUFFLE(2, 3, 1, 0));
	const __m128i l1 = _mm_max_epi32(min1, shuffle3);
	const __m128i l0 = _mm_min_epi32(min1, shuffle3);

	// Figure this out
	const __m128i unpack0 = _mm_unpackhi_epi32(l1, l3);
	const __m128i unpack1 = _mm_unpackhi_epi32(l0, l2);
	const __m128i result = _mm_unpackhi_epi32(unpack1, unpack0);

	const int visitOrderIndices[4u] =
	{
		_mm_cvtsi128_si32(result),
		_mm_cvtsi128_si32(_mm_shuffle_epi32(result, _MM_SHUFFLE(1, 1, 1, 1))),
		_mm_cvtsi128_si32(_mm_shuffle_epi32(result, _MM_SHUFFLE(2, 2, 2, 2))),
		_mm_cvtsi128_si32(_mm_shuffle_epi32(result, _MM_SHUFFLE(3, 3, 3, 3)))
	};

	const uint32_t intersectionCount = __popcnt(*((uint32_t*)&intersectionMask));
	for (uint32_t i = 0u; i < intersectionCount; i++)
	{
		const uint32_t visitIndex = (uint32_t)(visitOrderIndices[i] & 0x00000003);

		if (node.m_child[visitIndex] >> 31u)
		{
			const uint32_t triangle4Index = node.m_child[visitIndex] & ~(1u << 31u);
			const TraversalTriangle4& triangle4 = m_traversalDataManager->GetBVH4TraversalTriangle4(triangle4Index);

			HitTriangle4(ray, triangle4, triangle4Index, tMin, out_primitiveId, out_tMax, out_u, out_v, moveMask);
		}
		else
		{
			BVH4DFSTraversal<T_acceptAnyHit>(node.m_child[visitIndex], ray, tMin, out_primitiveId, out_tMax, out_u, out_v, moveMask);
		}

		if constexpr (T_acceptAnyHit)
		{
			if (moveMask)
			{
				return;
			}
		}
	}
}
