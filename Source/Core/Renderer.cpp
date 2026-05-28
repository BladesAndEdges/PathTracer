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
#include "Traversals.h"
#include "TraversalDataManager.h"
#include "TraversalTriangle.h"
#include "TriangleTexCoords.h"
#include "TriangleTexCoords4.h"

#include "Pathtracer.h"

# define M_PI 3.14159265358979323846
//#define TRACE_AGAINST_NON_BVH
//#define TRACE_AGAINST_NON_BVH_SSE
//#define TRACE_AGAINST_BVH2
#define TRACE_AGAINST_BVH4

// --------------------------------------------------------------------------------
Renderer::Renderer()
{
	m_isFirstFrame = true;

	m_sceneManager = new SceneManager("sponza.obj", 0.01f, "sponza.mtl");

	m_traversalDataManager = new TraversalDataManager(m_sceneManager->GetTriangles(), m_sceneManager->GetPerTriangleMaterials());

	m_camera.SetCameraLocation(m_sceneManager->GetInitialCameraPosition());

	m_lightDirection = Normalize(Vector3(0.9f, 1.0f, 0.4f));

	m_pathtracer = new Pathtracer();
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
		m_pathtracer->GenerateViewspaceDirections(framebuffer->GetWidth(), framebuffer->GetHeight());
		m_isFirstFrame = false;
	}

	std::vector<uint32_t> primaryRayAABBIntersectionsCount;
	std::vector<uint32_t> primaryRayTriangleIntersectionsCount;
	std::vector<uint32_t> primaryRayNodeVisits;

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
	const SHORT jKeyState = GetAsyncKeyState(0x4A);

	const Vector3 primitiveDebugColours[5u] = { Vector3(0.94f, 0.34f, 0.30f), Vector3(0.30f, 0.94f, 0.70f), Vector3(0.51f, 0.70f, 0.96f),
		Vector3(0.96f, 0.91f, 0.51f), Vector3(0.96f, 0.61f, 0.91f) };

	//----------------------------------------------------------------------------------------------------------------------------------------------------------------
	pc.BeginTiming();
	uint8_t* bytes = framebuffer->GetDataPtr();
	const std::vector<Vector3>& viewSpaceDirections = m_pathtracer->GetViewSpaceDirections();
	const float pixelHalfWidth = m_pathtracer->GetPixelWidth() / 2.0f;
	const float pixelHalfHeight = m_pathtracer->GetPixelHeight() / 2.0f;

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

			if ((cKeyState == 0u) && (vKeyState == 0u) && (bKeyState == 0u) && (nKeyState == 0u) && (mKeyState == 0u) && (xKeyState == 0u) && (lKeyState == 0u) && (kKeyState == 0u) && (jKeyState == 0u))
			{
				Vector3 radiance(0.0f, 0.0f, 0.0f);
				const uint32_t numSamples = 4u;
				const uint32_t depth = 4u;

				for (uint32_t sample = 0u; sample < numSamples; sample++)
				{
					Vector3 texelTopLeft;
					Vector3 texelBottomRight;
					

					texelTopLeft.SetX(viewSpaceDirections[rayIndex].X() - pixelHalfWidth);
					texelTopLeft.SetY(viewSpaceDirections[rayIndex].Y() + pixelHalfHeight);

					texelBottomRight.SetX(viewSpaceDirections[rayIndex].X() + pixelHalfWidth);
					texelBottomRight.SetY(viewSpaceDirections[rayIndex].Y() - pixelHalfHeight);

					const float randomX = RandomFloat(texelTopLeft.X(), texelBottomRight.X());
					const float randomY = RandomFloat(texelBottomRight.Y(), texelTopLeft.Y());

					Ray primaryRay(m_camera.GetCameraLocation(), Vector3(randomX, randomY, viewSpaceDirections[rayIndex].Z()));

					radiance = radiance + PathTrace(primaryRay, rayIndex, depth);
				}

				radiance = Vector3(radiance.X() / (float)numSamples, radiance.Y() / (float)numSamples, radiance.Z() / (float)numSamples);

				// Clamp prior to the conversion, assume SDR
				red = std::fmin(1.0f, radiance.X());
				green = std::fmin(1.0f, radiance.Y());
				blue = std::fmin(1.0f, radiance.Z());
			}
			else
			{
				Ray primaryRay(m_camera.GetCameraLocation(), viewSpaceDirections[rayIndex]);

				// Trace based on selected method
#ifdef TRACE_AGAINST_NON_BVH
				const HitResult hr = TraceRayNonBVH<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH_SSE
				const HitResult hr = TraceRay4NonBVH<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH2
				const HitResult hr = TraceAgainstBVH2<false>(primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH4
				const HitResult hr = TraceAgainstBVH4<false>(primaryRay, rayIndex, 1e-5f);
#endif

				// Check ray traversal/intersection statistics
				if (xKeyState > 0u)
				{
#ifdef TRACE_AGAINST_NON_BVH
					primaryRayTriangleIntersectionsCount[rayIndex] = (primaryRay.m_primaryTriangleIntersectionTests);
#endif
#ifdef TRACE_AGAINST_NON_BVH_SSE
					primaryRayTriangleIntersectionsCount[rayIndex] = (primaryRay.m_primaryTriangleIntersectionTests);
#endif
#ifdef TRACE_AGAINST_BVH2
					primaryRayAABBIntersectionsCount[rayIndex] = (primaryRay.m_primaryAABBIntersectionTests);
					primaryRayTriangleIntersectionsCount[rayIndex] = (primaryRay.m_primaryTriangleIntersectionTests);
					primaryRayNodeVisits[rayIndex] = primaryRay.m_primaryNodeVisits;
#endif
#ifdef TRACE_AGAINST_BVH4
					primaryRayAABBIntersectionsCount[rayIndex] = (primaryRay.m_primaryAABBIntersectionTests);
					primaryRayTriangleIntersectionsCount[rayIndex] = (primaryRay.m_primaryTriangleIntersectionTests);
					primaryRayNodeVisits[rayIndex] = primaryRay.m_primaryNodeVisits;
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

				// In shadow check
				if (cKeyState > 0u)
				{
					if (hr.m_t != INFINITY)
					{
						Ray shadowRay(hr.m_intersectionPoint, m_lightDirection);

#ifdef TRACE_AGAINST_NON_BVH
						const HitResult shadowHr = TraceRayNonBVH<true>(shadowRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH_SSE
						const HitResult shadowHr = TraceRay4NonBVH<true>(shadowRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH2
						const HitResult shadowHr = TraceAgainstBVH2<true>(shadowRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH4
						const HitResult shadowHr = TraceAgainstBVH4<true>(shadowRay, rayIndex, 1e-5f);
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
					const float maxDepth = 10.0f;

					red = (hr.m_t != INFINITY) ? hr.m_intersectionPoint.X() / maxDepth : 0.0f;
					green = (hr.m_t != INFINITY) ? hr.m_intersectionPoint.Y() / maxDepth : 0.0f;
					blue = (hr.m_t != INFINITY) ? hr.m_intersectionPoint.Z() / maxDepth : 0.0f;
				}

				if (bKeyState > 0u)
				{
					const float maxDepth = 10.0f;

					red = (hr.m_t != INFINITY) ? hr.m_t / maxDepth : 0.0f;
					green = (hr.m_t != INFINITY) ? hr.m_t / maxDepth : 0.0f;
					blue = (hr.m_t != INFINITY) ? hr.m_t / maxDepth : 0.0f;
				}

				// Normals
				if (nKeyState > 0u)
				{
					red = (hr.m_t < INFINITY) ? 0.5f * hr.m_normal.X() + 0.5f : 0.0f;
					green = (hr.m_t < INFINITY) ? 0.5f * hr.m_normal.Y() + 0.5f : 0.0f;
					blue = (hr.m_t < INFINITY) ? 0.5f * hr.m_normal.Z() + 0.5f : 0.0f;
				}

				// Primitive ids
				if (mKeyState > 0u)
				{
					red = (hr.m_primitiveId != UINT32_MAX) ? primitiveDebugColours[hr.m_primitiveId % 5u].X() : 0.0f;
					green = (hr.m_primitiveId != UINT32_MAX) ? primitiveDebugColours[hr.m_primitiveId % 5u].Y() : 0.0f;
					blue = (hr.m_primitiveId != UINT32_MAX) ? primitiveDebugColours[hr.m_primitiveId % 5u].Z() : 0.0f;
				}

				// Material ids
				if (lKeyState > 0u)
				{
					red = (hr.m_materialId != UINT32_MAX) ? m_sceneManager->GetDebugMaterialColour(hr.m_materialId).X() : 0.0f;
					green = (hr.m_materialId != UINT32_MAX) ? m_sceneManager->GetDebugMaterialColour(hr.m_materialId).Y() : 0.0f;
					blue = (hr.m_materialId != UINT32_MAX) ? m_sceneManager->GetDebugMaterialColour(hr.m_materialId).Z() : 0.0f;
				}

				// Texture coordinates
				if (kKeyState > 0u)
				{
					// Possibly do some other comparison due to fp precision
					const bool uInRange = ((hr.m_texCoords.X() >= 0.0f) && (hr.m_texCoords.X() <= 1.0f));
					const bool vInRange = ((hr.m_texCoords.Y() >= 0.0f) && (hr.m_texCoords.Y() <= 1.0f));

					red = (uInRange && vInRange) ? hr.m_texCoords.X() : 1.0f;
					green = (uInRange && vInRange) ? hr.m_texCoords.Y() : 0.75f;
					blue = (uInRange && vInRange) ? 0.0f : 0.8f;
				}

				// Surface colour
				if (jKeyState > 0u)
				{
					red = hr.m_colour.X();
					green = hr.m_colour.Y();
					blue = hr.m_colour.Z();
				}
			}

			bytes[texelByteIndex] = uint8_t((red * 255.0f) + 0.5f);
			bytes[texelByteIndex + 1u] = uint8_t((green * 255.0f) + 0.5f);
			bytes[texelByteIndex + 2u] = uint8_t((blue * 255.0f) + 0.5f);
			bytes[texelByteIndex + 3u] = 255u;
		}
	}

	pc.EndTiming();

	char buffer[128];
	sprintf_s(buffer, "Total frame time: %f \n", pc.GetMilliseconds());
	OutputDebugStringA(buffer);
}

// --------------------------------------------------------------------------------
Vector3 Renderer::PathTrace(Ray& ray, const uint32_t rayIndex, uint32_t depth)
{
	if (depth <= 0u)
	{
		return Vector3(0.0f, 0.0f, 0.0f);
	}

	Vector3 radiance(0.0f, 0.0f, 0.0f);

#ifdef TRACE_AGAINST_NON_BVH
	const HitResult c_primaryHitResult = TraceRayNonBVH<false>(ray, rayIndex, 1e-5f);
#endif 
#ifdef TRACE_AGAINST_NON_BVH_SSE
	const HitResult c_primaryHitResult = TraceRay4NonBVH<false>(ray, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH2
	const HitResult c_primaryHitResult = TraceAgainstBVH2<false>(ray, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH4
	const HitResult c_primaryHitResult = TraceAgainstBVH4<false>(ray, rayIndex, 1e-5f);
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
			
			// Divide everything by the probability distribution function, for our case just 1/ 2 * Pi
			const float pdf = 1.0f / (2.0f * (float)M_PI);
			radiance = Vector3(radiance.X() / pdf, radiance.Y() / pdf, radiance.Z() / pdf);
		}

		//Direct Lighting
		{
			const float clampValue = std::fmin(std::fmax(Dot(m_lightDirection, c_primaryHitResult.m_normal), 0.0f), 1.0f);

			Ray c_shadowRay(c_primaryHitResult.m_intersectionPoint, m_lightDirection);

#ifdef TRACE_AGAINST_NON_BVH
			const HitResult c_secondaryRayHitResult = TraceRayNonBVH<true>(c_shadowRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH_SSE
			const HitResult c_secondaryRayHitResult = TraceRay4NonBVH<true>(c_shadowRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH2
			const HitResult c_secondaryRayHitResult = TraceAgainstBVH2<true>(c_shadowRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH4
			const HitResult c_secondaryRayHitResult = TraceAgainstBVH4<true>(c_shadowRay, rayIndex, 1e-5f);
#endif

			if (c_secondaryRayHitResult.m_t == INFINITY)
			{
				Vector3 directRadiance;
				directRadiance.SetX(clampValue * c_primaryHitResult.m_colour.X());
				directRadiance.SetY(clampValue * c_primaryHitResult.m_colour.Y());
				directRadiance.SetZ(clampValue * c_primaryHitResult.m_colour.Z());

				radiance = radiance + 2.0 * directRadiance;
			}
		}
	}
	else
	{
		const float skyIntensity = 1.5f;
		const Vector3 skyColour = skyIntensity * Vector3(0.98f, 0.68f, 0.37f);
		
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

		const Vector3 edge1 = Normalize(Vector3(traversalTriangles[primitiveId].m_edge1[0u], traversalTriangles[primitiveId].m_edge1[1u], traversalTriangles[primitiveId].m_edge1[2u]));
		const Vector3 edge2 = Normalize(Vector3(traversalTriangles[primitiveId].m_edge2[0u], traversalTriangles[primitiveId].m_edge2[1u], traversalTriangles[primitiveId].m_edge2[2u]));
		const Vector3 normal = Normalize(Cross(edge1, edge2));
		hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;

		hitResult.m_primitiveId = primitiveId;

		hitResult.m_materialId = materialIndices[primitiveId];

		hitResult.m_colour = m_sceneManager->BasicSample(materialIndices[primitiveId], hitResult.m_texCoords.X(), hitResult.m_texCoords.Y());
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

		const Vector3 edge1 = Normalize(Vector3(traversalTriangle4s[tri4Index].m_edge1X[subIndex], 
			traversalTriangle4s[tri4Index].m_edge1Y[subIndex], 
			traversalTriangle4s[tri4Index].m_edge1Z[subIndex]));

		const Vector3 edge2 = Normalize(Vector3(traversalTriangle4s[tri4Index].m_edge2X[subIndex], 
			traversalTriangle4s[tri4Index].m_edge2Y[subIndex], 
			traversalTriangle4s[tri4Index].m_edge2Z[subIndex]));

		const Vector3 normal = Normalize(Cross(edge1, edge2));
		hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;

		hitResult.m_primitiveId = (tri4Index * 4u) + subIndex;

		const std::vector<Material4Index>& material4Indices = m_traversalDataManager->GetMaterial4Indices();
		hitResult.m_materialId = material4Indices[tri4Index].m_indices[subIndex];

		hitResult.m_colour = m_sceneManager->BasicSample(hitResult.m_materialId, hitResult.m_texCoords.X(), hitResult.m_texCoords.Y());
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

	BVH2Traversal<T_acceptAnyHit>(m_traversalDataManager, 0u, ray, tMin, primitiveId, tMax, tu, tv, hasHit);
	
	if (hasHit)
	{
		hitResult.m_t = tMax;

		hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(tMax);


		const TriangleTexCoords& texCoords = m_traversalDataManager->GetBVH2TriangleTexCoords(primitiveId);
		hitResult.m_texCoords.SetX((1.0f - tu - tv) * texCoords.m_v0uv[0u] + tu * texCoords.m_v1uv[0u] + tv * texCoords.m_v2uv[0u]);
		hitResult.m_texCoords.SetY((1.0f - tu - tv) * texCoords.m_v0uv[1u] + tu * texCoords.m_v1uv[1u] + tv * texCoords.m_v2uv[1u]);

		const TraversalTriangle& traversalTriangle = m_traversalDataManager->GetBVH2TraversalTriangle(primitiveId);
		const Vector3 edge1 = Normalize(Vector3(traversalTriangle.m_edge1[0u], traversalTriangle.m_edge1[1u], traversalTriangle.m_edge1[2u]));
		const Vector3 edge2 = Normalize(Vector3(traversalTriangle.m_edge2[0u], traversalTriangle.m_edge2[1u], traversalTriangle.m_edge2[2u]));
		const Vector3 normal = Normalize(Cross(edge1, edge2));
		hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;

		hitResult.m_primitiveId = primitiveId;

		hitResult.m_materialId = m_traversalDataManager->GetBVH2MaterialIndex(primitiveId);

		hitResult.m_colour = m_sceneManager->BasicSample(hitResult.m_materialId, hitResult.m_texCoords.X(), hitResult.m_texCoords.Y());
	}

	return hitResult;
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

	BVH4Traversal<T_acceptAnyHit>(m_traversalDataManager, 0u, ray, tMin, outTri4Indices, outTMax, outU, outV, moveMask);

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

		const TraversalTriangle4& traversalTriangle4 = m_traversalDataManager->GetBVH4TraversalTriangle4(tri4Index);
		const Vector3 edge1 = Normalize(Vector3(traversalTriangle4.m_edge1X[subIndex], traversalTriangle4.m_edge1Y[subIndex], traversalTriangle4.m_edge1Z[subIndex]));
		const Vector3 edge2 = Normalize(Vector3(traversalTriangle4.m_edge2X[subIndex], traversalTriangle4.m_edge2Y[subIndex], traversalTriangle4.m_edge2Z[subIndex]));
		const Vector3 normal = Normalize(Cross(edge1, edge2));
		hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;

		const TriangleIndices& triangleIndices = m_traversalDataManager->GetBVH4TriangleIndices(tri4Index);
		hitResult.m_primitiveId = triangleIndices.m_triangleIndices[subIndex];

		const Material4Index& material4Index = m_traversalDataManager->GetBVH4Material4Index(tri4Index);
		hitResult.m_materialId = material4Index.m_indices[subIndex];

		hitResult.m_colour = m_sceneManager->BasicSample(material4Index.m_indices[subIndex], hitResult.m_texCoords.X(), hitResult.m_texCoords.Y());
	}

	return hitResult;
}
