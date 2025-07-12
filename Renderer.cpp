#include "Renderer.h"

#include <assert.h>
#include <emmintrin.h>
#include <xmmintrin.h>
#include <immintrin.h>

#include "BVH2AccellStructure.h"
#include "BVH4AccellStructure.h"
#include "Framebuffer.h"
#include "PerformanceCounter.h"
#include "Vector3.h"

# define M_PI 3.14159265358979323846
//#define TRACE_AGAINST_BVH2
#define TRACE_AGAINST_BVH4
//#define TRACE_AGAINST_NON_BVH
//#define RUNNING_SCALAR

// --------------------------------------------------------------------------------
Renderer::Renderer(const std::vector<float>& positionsX, const std::vector<float>& positionsY, const std::vector<float>& positionsZ,
	const std::vector<Triangle4>& triangle4s, const std::vector<Triangle>& triangles, const Vector3& center)	
																										: m_positionsX(positionsX), 
																										  m_positionsY(positionsY),
																										  m_positionsZ(positionsZ),
																										  m_triangle4s(triangle4s),
																										  m_triangles(triangles),
																										  m_center(center)
{	
	m_center = Vector3(2.88791323f, 7.37331104f, -0.183363333f); // For sponza


	m_camera.SetCameraLocation(m_center);
	m_lightDirection = Normalize(Vector3(1.0f, 1.0f, 1.0f));

	ZeroMemory((void*)&m_viewportDesc, sizeof(m_viewportDesc));
	m_isFirstFrame = true;
	
	// testing/sanity check
#ifdef _DEBUG
	uint32_t currentTri4Id = 0u;
	for (uint32_t triangleOffset = 0u; triangleOffset < m_positionsX.size(); triangleOffset+=3u)
	{
		const Vector3 edge1 = Vector3(m_positionsX[triangleOffset + 1u] - m_positionsX[triangleOffset],
			m_positionsY[triangleOffset + 1u] - m_positionsY[triangleOffset],
			m_positionsZ[triangleOffset + 1u] - m_positionsZ[triangleOffset]);
	
		const Vector3 edge2 = Vector3(m_positionsX[triangleOffset + 2u] - m_positionsX[triangleOffset],
			m_positionsY[triangleOffset + 2u] - m_positionsY[triangleOffset],
			m_positionsZ[triangleOffset + 2u] - m_positionsZ[triangleOffset]);
	
		const Vector3 v0 = Vector3(m_positionsX[triangleOffset], m_positionsY[triangleOffset], m_positionsZ[triangleOffset]);
	
		if ((triangleOffset != 0u) && ((triangleOffset % 12u) == 0u))
		{
			currentTri4Id++;
		}

		const uint32_t currentTriId = (triangleOffset % 12u) / 3u;
	
		assert(m_triangle4s[currentTri4Id].m_edge1X[currentTriId] == edge1.X());
		assert(m_triangle4s[currentTri4Id].m_edge1Y[currentTriId] == edge1.Y());
		assert(m_triangle4s[currentTri4Id].m_edge1Z[currentTriId] == edge1.Z());
		
		assert(m_triangle4s[currentTri4Id].m_edge2X[currentTriId] == edge2.X());
		assert(m_triangle4s[currentTri4Id].m_edge2Y[currentTriId] == edge2.Y());
		assert(m_triangle4s[currentTri4Id].m_edge2Z[currentTriId] == edge2.Z());
		
		assert(m_triangle4s[currentTri4Id].m_v0X[currentTriId] == v0.X());
		assert(m_triangle4s[currentTri4Id].m_v0Y[currentTriId] == v0.Y());
		assert(m_triangle4s[currentTri4Id].m_v0Z[currentTriId] == v0.Z());
	}
#endif

	m_bvh2AccellStructure = new BVH2AccellStructure(m_triangles, BVH2PartitionStrategy::HalfWayLongestAxisWithSAH);
	m_bvh4AccellStructure = new BVH4AccellStructure(m_bvh2AccellStructure);
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

	std::vector<uint32_t> aabbIntersectionsCount;
	std::vector<uint32_t> triangleIntersectionsCount;
	std::vector<uint32_t> nodeVisits;
	aabbIntersectionsCount.resize(framebuffer->GetWidth() * framebuffer->GetHeight());
	triangleIntersectionsCount.resize(framebuffer->GetWidth() * framebuffer->GetHeight());
	nodeVisits.resize(framebuffer->GetWidth() * framebuffer->GetHeight());

	const SHORT cKeyState = GetAsyncKeyState(0x43);
	const SHORT vKeyState = GetAsyncKeyState(0x56);
	const SHORT bKeyState = GetAsyncKeyState(0x42);
	const SHORT nKeyState = GetAsyncKeyState(0x4E);
	const SHORT mKeyState = GetAsyncKeyState(0x4D);
	const SHORT xKeyState = GetAsyncKeyState(0x58);

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

			if ((cKeyState == 0u) && (vKeyState == 0u) && (bKeyState == 0u) && (nKeyState == 0u) && (mKeyState == 0u) && (xKeyState == 0u))
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
			pc.EndTiming();

			if (xKeyState > 0u)
			{
				// Ray values reset here, so no need to do a reset on the vector
				Ray c_primaryRay(m_camera.GetCameraLocation(), m_texelCenters[rayIndex]);

#ifdef TRACE_AGAINST_BVH2
				const HitResult hr = TraceAgainstBVH2<false>(c_primaryRay, rayIndex, 1e-5f);
				aabbIntersectionsCount[rayIndex] = (c_primaryRay.m_aabbIntersectionTests);
				triangleIntersectionsCount[rayIndex] = (c_primaryRay.m_triangleIntersectionTests);
				nodeVisits[rayIndex] = c_primaryRay.m_nodeVisits;
#endif
#ifdef TRACE_AGAINST_BVH4
				const HitResult hr = TraceAgainstBVH4<false>(c_primaryRay, rayIndex, 1e-5f);
				aabbIntersectionsCount[rayIndex] = (c_primaryRay.m_aabbIntersectionTests);
				triangleIntersectionsCount[rayIndex] = (c_primaryRay.m_triangleIntersectionTests);
				nodeVisits[rayIndex] = c_primaryRay.m_nodeVisits;
#endif
#ifdef TRACE_AGAINST_NON_BVH
				const HitResult hr = TraceRay<false>(c_primaryRay, rayIndex, 1e-5f);
				triangleIntersectionsCount[rayIndex] = (c_primaryRay.m_triangleIntersectionTests);
#endif // !TRACE_AGAINST_NON_BVH

				if ((row == framebuffer->GetHeight() - 1u) && (column == framebuffer->GetWidth() - 1u))
				{
					// Average aabb intersections for primary rays
					uint32_t averageAABBVisits = 0u;
					for (uint32_t i = 0u; i < aabbIntersectionsCount.size(); i++)
					{
						averageAABBVisits += aabbIntersectionsCount[i];
					}

					averageAABBVisits = averageAABBVisits / (uint32_t)aabbIntersectionsCount.size();

					char msgBuffer1[128u];
					sprintf_s(msgBuffer1, "Average AABB visits: %u \n", averageAABBVisits);
					OutputDebugStringA(msgBuffer1);


					// Average triangle intersections for primary rays
					uint32_t averageTriangleVisits = 0u;
					for (uint32_t i = 0u; i < triangleIntersectionsCount.size(); i++)
					{
						averageTriangleVisits += triangleIntersectionsCount[i];
					}

					averageTriangleVisits = averageTriangleVisits / (uint32_t)triangleIntersectionsCount.size();

					char msgBuffer2[128u];
					sprintf_s(msgBuffer2, "Average Triangle visits: %u \n", averageTriangleVisits);
					OutputDebugStringA(msgBuffer2);

					// Average node visits
					uint32_t averageNodeVisits = 0u;
					for (uint32_t i = 0u; i < nodeVisits.size(); i++)
					{
						averageNodeVisits += nodeVisits[i];
					}

					averageNodeVisits = averageNodeVisits / (uint32_t)nodeVisits.size();

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
				Ray c_primaryRay(m_camera.GetCameraLocation(), m_texelCenters[rayIndex]);
				
#ifdef TRACE_AGAINST_BVH2
				const HitResult primaryHr = TraceAgainstBVH2<false>(c_primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH4
				const HitResult primaryHr = TraceAgainstBVH4<false>(c_primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH
				const HitResult primaryHr = TraceRay<false>(c_primaryRay, rayIndex, 1e-5f);
#endif // !TRACE_AGAINST_BVH

				//const float clampValue = std::fmin(std::fmax(Dot(m_lightDirection, primaryHr.m_normal), 0.0f), 1.0f);

				if (primaryHr.m_t != INFINITY)
				{
					Ray c_shadowRay(primaryHr.m_intersectionPoint, m_lightDirection);
					
#ifdef TRACE_AGAINST_BVH2
					const HitResult shadowHr = TraceAgainstBVH2<false>(c_shadowRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH4
					const HitResult shadowHr = TraceAgainstBVH4<false>(c_shadowRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH
					const HitResult shadowHr = TraceRay<false>(c_shadowRay, rayIndex, 1e-5f);
#endif // !TRACE_AGAINST_BVH

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
				Ray c_primaryRay(m_camera.GetCameraLocation(), m_texelCenters[rayIndex]);
				
#ifdef TRACE_AGAINST_BVH2
				const HitResult hr = TraceAgainstBVH2<false>(c_primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH4
				const HitResult hr = TraceAgainstBVH4<false>(c_primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH
				const HitResult hr = TraceRay<false>(c_primaryRay, rayIndex, 1e-5f);
#endif // !TRACE_AGAINST_BVH

				const float maxDepth = 10.0f;

				red = (hr.m_t != INFINITY) ? hr.m_intersectionPoint.X() / maxDepth : 0.0f;
				green = (hr.m_t != INFINITY) ? hr.m_intersectionPoint.Y() / maxDepth : 0.0f;
				blue = (hr.m_t != INFINITY) ? hr.m_intersectionPoint.Z() / maxDepth : 0.0f;
			}

			if (bKeyState > 0u)
			{
				Ray c_primaryRay(m_camera.GetCameraLocation(), m_texelCenters[rayIndex]);
				
#ifdef TRACE_AGAINST_BVH2
				const HitResult hr = TraceAgainstBVH2<false>(c_primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH4
				const HitResult hr = TraceAgainstBVH4<false>(c_primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH
				const HitResult hr = TraceRay<false>(c_primaryRay, rayIndex, 1e-5f);
#endif // !TRACE_AGAINST_BVH

				const float maxDepth = 10.0f;

				red = (hr.m_t != INFINITY) ? hr.m_t / maxDepth : 0.0f;
				green = (hr.m_t != INFINITY) ? hr.m_t / maxDepth : 0.0f;
				blue = (hr.m_t != INFINITY) ? hr.m_t / maxDepth : 0.0f;
			}

			if (nKeyState > 0u)
			{
				Ray c_primaryRay(m_camera.GetCameraLocation(), m_texelCenters[rayIndex]);
				
#ifdef TRACE_AGAINST_BVH2
				const HitResult hr = TraceAgainstBVH2<false>(c_primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH4
				const HitResult hr = TraceAgainstBVH4<false>(c_primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH
				const HitResult hr = TraceRay<false>(c_primaryRay, rayIndex, 1e-5f);
#endif // !TRACE_AGAINST_BVH

				red = (hr.m_t < INFINITY) ? 0.5f * hr.m_normal.X() + 0.5f : 0.0f;
				green = (hr.m_t < INFINITY) ? 0.5f * hr.m_normal.Y() + 0.5f : 0.0f;
				blue = (hr.m_t < INFINITY) ? 0.5f * hr.m_normal.Z() + 0.5f : 0.0f;
			}

			if (mKeyState > 0u)
			{
				Ray c_primaryRay(m_camera.GetCameraLocation(), m_texelCenters[rayIndex]);

#ifdef TRACE_AGAINST_BVH2
				const HitResult hr = TraceAgainstBVH2<false>(c_primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_BVH4
				const HitResult hr = TraceAgainstBVH4<false>(c_primaryRay, rayIndex, 1e-5f);
#endif
#ifdef TRACE_AGAINST_NON_BVH
				const HitResult hr = TraceRay<false>(c_primaryRay, rayIndex, 1e-5f);
#endif // !TRACE_AGAINST_BVH

				red = (hr.m_primitiveId != UINT32_MAX) ? primitiveDebugColours[hr.m_primitiveId % 5u].X() : 0.0f;
				green = (hr.m_primitiveId != UINT32_MAX) ? primitiveDebugColours[hr.m_primitiveId % 5u].Y() : 0.0f;
				blue = (hr.m_primitiveId != UINT32_MAX) ? primitiveDebugColours[hr.m_primitiveId % 5u].Z() : 0.0f;
			}

			bytes[texelByteIndex] = uint8_t(red * 255.0f);
			bytes[texelByteIndex + 1u] = uint8_t(green * 255.0f);
			bytes[texelByteIndex + 2u] = uint8_t(blue * 255.0f);
			bytes[texelByteIndex + 3u] = 1u;
		}
	}
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

//#define RUNNING_SCALAR_TRI4
//#define RUNNING_SSE
//#define RUNNING_SCALAR_WITHOUT_FACES
#define RUNNING_SCALAR_WITH_FACES

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
void Renderer::HitTriangles(Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult)
{
#ifdef _DEBUG
	assert(rayIndex >= 0u);
#endif
#ifdef NDEBUG
	(void)(rayIndex);
#endif

#ifdef RUNNING_SSE
	// Constant registers
	const __m128 c_epsilon = _mm_set1_ps(1e-8f);
	const __m128 c_allZeros = _mm_set1_ps(0.0f);
	const __m128 c_allOnes = _mm_set1_ps(1.0f);

	// Ray data
	const __m128 rayOriginX = _mm_set1_ps(ray.Origin().X());
	const __m128 rayOriginY = _mm_set1_ps(ray.Origin().Y());
	const __m128 rayOriginZ = _mm_set1_ps(ray.Origin().Z());

	const __m128 rayDirectionX = _mm_set1_ps(ray.Direction().X());
	const __m128 rayDirectionY = _mm_set1_ps(ray.Direction().Y());
	const __m128 rayDirectionZ = _mm_set1_ps(ray.Direction().Z());

	const __m128 tMinimum = _mm_set1_ps(tMin);
	const __m128 tMaximum = _mm_set1_ps(tMax);

	// Output global smallest Ts
	__m128 smallestTs = _mm_set1_ps(INFINITY);
	__m128i smallestPrimitiveIds = _mm_set1_epi32(-1);

	// Loop variables
	const __m128i c_incrementRegister = _mm_set1_epi32(4);
	__m128i primitiveIds = _mm_set_epi32(3, 2, 1, 0);

	for (uint32_t currentTri4 = 0u; currentTri4 < m_triangle4s.size(); currentTri4++)
	{
		// Load tri4 data
		const __m128 edge1X = _mm_loadu_ps(m_triangle4s[currentTri4].m_edge1X.data());
		const __m128 edge1Y = _mm_loadu_ps(m_triangle4s[currentTri4].m_edge1Y.data());
		const __m128 edge1Z = _mm_loadu_ps(m_triangle4s[currentTri4].m_edge1Z.data());

		const __m128 edge2X = _mm_loadu_ps(m_triangle4s[currentTri4].m_edge2X.data());
		const __m128 edge2Y = _mm_loadu_ps(m_triangle4s[currentTri4].m_edge2Y.data());
		const __m128 edge2Z = _mm_loadu_ps(m_triangle4s[currentTri4].m_edge2Z.data());

		const __m128 v0X = _mm_loadu_ps(m_triangle4s[currentTri4].m_v0X.data());
		const __m128 v0Y = _mm_loadu_ps(m_triangle4s[currentTri4].m_v0Y.data());
		const __m128 v0Z = _mm_loadu_ps(m_triangle4s[currentTri4].m_v0Z.data());

		// Calculate pvec
		const __m128 pvecXLHS = _mm_mul_ps(rayDirectionY, edge2Z);
		const __m128 pvecXRHS = _mm_mul_ps(rayDirectionZ, edge2Y);
		const __m128 pvecX = _mm_sub_ps(pvecXLHS, pvecXRHS);

		const __m128 pvecYLHS = _mm_mul_ps(rayDirectionZ, edge2X);
		const __m128 pvecYRHS = _mm_mul_ps(rayDirectionX, edge2Z);
		const __m128 pvecY = _mm_sub_ps(pvecYLHS, pvecYRHS);

		const __m128 pvecZLHS = _mm_mul_ps(rayDirectionX, edge2Y);
		const __m128 pvecZRHS = _mm_mul_ps(rayDirectionY, edge2X);
		const __m128 pvecZ = _mm_sub_ps(pvecZLHS, pvecZRHS);

		// Calculate determinants
		const __m128 detDotX = _mm_mul_ps(pvecX, edge1X); // fmadd: _mm_fmadd_ps(), header is already added
		const __m128 detDotY = _mm_mul_ps(pvecY, edge1Y);
		const __m128 detDotZ = _mm_mul_ps(pvecZ, edge1Z);

		const __m128 detAddXY = _mm_add_ps(detDotX, detDotY);
		const __m128 determinants = _mm_add_ps(detAddXY, detDotZ);

		const __m128 signMask = _mm_set1_ps(-0.0f);
		const __m128 absDeterminants = _mm_andnot_ps(signMask, determinants);

		// Calculate intersection mask
		const __m128 hasIntersectedMask = _mm_cmpge_ps(absDeterminants, c_epsilon); // mask

		// Calculate inverse determinant
		const __m128 invDeterminant = _mm_div_ps(c_allOnes, determinants);
		
		// Calculate tvec
		const __m128 tvecX = _mm_sub_ps(rayOriginX, v0X);
		const __m128 tvecY = _mm_sub_ps(rayOriginY, v0Y);
		const __m128 tvecZ = _mm_sub_ps(rayOriginZ, v0Z);
		
		// Calculate u
		const __m128 uDotX = _mm_mul_ps(tvecX, pvecX);
		const __m128 uDotY = _mm_mul_ps(tvecY, pvecY);
		const __m128 uDotZ = _mm_mul_ps(tvecZ, pvecZ);
		
		const __m128 uAddXY = _mm_add_ps(uDotX, uDotY);
		const __m128 uAddFinal = _mm_add_ps(uAddXY, uDotZ);
		const __m128 u = _mm_mul_ps(uAddFinal, invDeterminant);
		
		// Calculate u mask
		const __m128 uIsGreaterEqual0 = _mm_cmpge_ps(u, c_allZeros);
		const __m128 uIsLessEqual1 = _mm_cmple_ps(u, c_allOnes);
		const __m128 isUValidMask = _mm_and_ps(uIsGreaterEqual0, uIsLessEqual1); // mask
		
		// Calculate qvec
		const __m128 qvecXLHS = _mm_mul_ps(tvecY, edge1Z);
		const __m128 qvecXRHS = _mm_mul_ps(tvecZ, edge1Y);
		const __m128 qvecX = _mm_sub_ps(qvecXLHS, qvecXRHS);
		
		const __m128 qvecYLHS = _mm_mul_ps(tvecZ, edge1X);
		const __m128 qvecYRHS = _mm_mul_ps(tvecX, edge1Z);
		const __m128 qvecY = _mm_sub_ps(qvecYLHS, qvecYRHS);
		
		const __m128 qvecZLHS = _mm_mul_ps(tvecX, edge1Y);
		const __m128 qvecZRHS = _mm_mul_ps(tvecY, edge1X);
		const __m128 qvecZ = _mm_sub_ps(qvecZLHS, qvecZRHS);
		
		// Calculate v
		const __m128 vDotX = _mm_mul_ps(rayDirectionX, qvecX);
		const __m128 vDotY = _mm_mul_ps(rayDirectionY, qvecY);
		const __m128 vDotZ = _mm_mul_ps(rayDirectionZ, qvecZ);
		
		const __m128 vAddXY = _mm_add_ps(vDotX, vDotY);
		const __m128 vAddFinal = _mm_add_ps(vAddXY, vDotZ);
		const __m128 v = _mm_mul_ps(vAddFinal, invDeterminant);
		
		// Calculate v masks
		const __m128 vIsGreaterEqual0 = _mm_cmpge_ps(v, c_allZeros);
		
		const __m128 uAddV = _mm_add_ps(u, v);
		const __m128 uAddVLessEqual1 = _mm_cmple_ps(uAddV, c_allOnes);
		
		const __m128 isInsideTriangleMask = _mm_and_ps(vIsGreaterEqual0, uAddVLessEqual1); // mask
		
		// Calculate t
		const __m128 tDotX = _mm_mul_ps(edge2X, qvecX);
		const __m128 tDotY = _mm_mul_ps(edge2Y, qvecY);
		const __m128 tDotZ = _mm_mul_ps(edge2Z, qvecZ);
		
		const __m128 tAddXY = _mm_add_ps(tDotX, tDotY);
		const __m128 tAddFinal = _mm_add_ps(tAddXY, tDotZ);
		const __m128 t = _mm_mul_ps(tAddFinal, invDeterminant);
		
		// Between tMin and tMax masks
		const __m128 tMoreThanTMin = _mm_cmpge_ps(t, tMinimum);
		const __m128 tLessThanTMax = _mm_cmple_ps(t, tMaximum);
		const __m128 tCheckMask = _mm_and_ps(tMoreThanTMin, tLessThanTMax);
		
		// Validity mask
		const __m128 tValidityMask = _mm_and_ps(_mm_and_ps(_mm_and_ps(hasIntersectedMask, isUValidMask), isInsideTriangleMask), tCheckMask);
		
		// Valid local values
		const __m128 validLocalTs = _mm_or_ps(_mm_and_ps(tValidityMask, t), _mm_andnot_ps(tValidityMask, tMaximum));

		const __m128i isSmallestInLaneMask = _mm_castps_si128(_mm_cmplt_ps(validLocalTs, smallestTs));

		// This changes to 7 as the mask thinks it should in fact change it for whatever reason
		const __m128i primIdsToChange = _mm_and_si128(isSmallestInLaneMask, primitiveIds);
		const __m128i primIdsToKeep = _mm_andnot_si128(isSmallestInLaneMask, smallestPrimitiveIds);
		smallestPrimitiveIds = _mm_or_si128(primIdsToChange, primIdsToKeep);

		// Update smallest ts
		smallestTs = _mm_min_ps(smallestTs, validLocalTs);

		if (T_acceptAnyHit) // This runs all the time for the sse version, the T_AcceptAnyHit runs only if the hit triangle returns something for the non-sse
		{
			if (_mm_movemask_ps(tValidityMask))
			{
				break;
			}
		}

		// Update increment values
		primitiveIds = _mm_add_epi32(primitiveIds, c_incrementRegister);
	}

	// Smallest T and it's primitive id in the XY lanes
	const __m128 minTShuffleXY = _mm_shuffle_ps(smallestTs, smallestTs, _MM_SHUFFLE(0, 0, 2, 3));
	const __m128 minTComparisonXY = _mm_min_ps(smallestTs, minTShuffleXY);

	// Smallest T and it's primitive Id now sotred in the 0th lane
	const __m128 minTShuffleX = _mm_shuffle_ps(minTComparisonXY, minTComparisonXY, _MM_SHUFFLE(0, 0, 0, 1));
	const __m128 minTComparisonX = _mm_min_ps(minTComparisonXY, minTShuffleX);

	const __m128i primIdShuffleXY = _mm_shuffle_epi32(smallestPrimitiveIds, _MM_SHUFFLE(0, 0, 2, 3));
	const __m128i primIdMaskXY = _mm_castps_si128(_mm_cmplt_ps(minTShuffleXY, smallestTs));
	const __m128i primIdToKeepXY = _mm_or_si128(_mm_and_si128(primIdMaskXY, primIdShuffleXY), _mm_andnot_si128(primIdMaskXY, smallestPrimitiveIds));

	const __m128i primIdShuffleX = _mm_shuffle_epi32(primIdToKeepXY, _MM_SHUFFLE(0, 0, 0, 1));
	const __m128i primIdMaskX = _mm_castps_si128(_mm_cmplt_ps(minTShuffleX, minTComparisonXY));
	const __m128i primIdtoKeepX = _mm_or_si128(_mm_and_si128(primIdMaskX, primIdShuffleX), _mm_andnot_si128(primIdMaskX, primIdToKeepXY));

	const int primitiveId = _mm_cvtsi128_si32(primIdtoKeepX);
	if (primitiveId > -1)
	{
		tMax = _mm_cvtss_f32(minTComparisonX);

		out_hitResult.m_t = _mm_cvtss_f32(minTComparisonX);

		out_hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(out_hitResult.m_t);
		out_hitResult.m_colour = Vector3(1.0f, 0.55f, 0.0f);;

		out_hitResult.m_primitiveId = primitiveId;

		const uint32_t tri4Id = primitiveId / 4u;
		const uint32_t triId = primitiveId % 4;
		const Vector3 edge1 = Vector3(m_triangle4s[tri4Id].m_edge1X[triId],
			m_triangle4s[tri4Id].m_edge1Y[triId],
			m_triangle4s[tri4Id].m_edge1Z[triId]);

		const Vector3 edge2 = Vector3(m_triangle4s[tri4Id].m_edge2X[triId],
			m_triangle4s[tri4Id].m_edge2Y[triId],
			m_triangle4s[tri4Id].m_edge2Z[triId]);

		const Vector3 normal = Normalize(Cross(edge1, edge2));

		out_hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;
	}
#endif
#ifdef RUNNING_SCALAR_TRI4

	std::vector<float> smallestTs(4u, INFINITY);
	std::vector<uint32_t> tri4Ids(4u, UINT32_MAX);

	std::vector<float> pvecX(4u);
	std::vector<float> pvecY(4u);
	std::vector<float> pvecZ(4u);

	std::vector<float> tvecX(4u);
	std::vector<float> tvecY(4u);
	std::vector<float> tvecZ(4u);

	std::vector<float> qvecX(4u);
	std::vector<float> qvecY(4u);
	std::vector<float> qvecZ(4u);

	std::vector<float> u(4u);
	std::vector<float> v(4u);

	std::vector<float> normalX(4u);
	std::vector<float> normalY(4u);
	std::vector<float> normalZ(4u);

	std::vector<float> determinants(4u);

	std::vector<Vector3> potentialNormals(4u);
	std::vector<Vector3> potentialColours(4u);
	std::vector<Vector3> testNormals(4u);

	bool hitFound = false;

	for (uint32_t currentTri4 = 0u; currentTri4 < m_triangle4s.size(); currentTri4++)
	{
		if (hitFound)
		{
			break;
		}

		// pvecX
		for (uint32_t currentX = 0u; currentX < m_triangle4s[currentTri4].m_edge2Z.size(); currentX++)
		{
			pvecX[currentX] = ray.Direction().Y() * m_triangle4s[currentTri4].m_edge2Z[currentX] -
				ray.Direction().Z() * m_triangle4s[currentTri4].m_edge2Y[currentX];
		}

		// pvecY
		for (uint32_t currentY = 0u; currentY < m_triangle4s[currentTri4].m_edge2X.size(); currentY++)
		{
			pvecY[currentY] = ray.Direction().Z() * m_triangle4s[currentTri4].m_edge2X[currentY] -
				ray.Direction().X() * m_triangle4s[currentTri4].m_edge2Z[currentY];
		}

		// pvecZ
		for (uint32_t currentZ = 0u; currentZ < m_triangle4s[currentTri4].m_edge2Y.size(); currentZ++)
		{
			pvecZ[currentZ] = ray.Direction().X() * m_triangle4s[currentTri4].m_edge2Y[currentZ] -
				ray.Direction().Y() * m_triangle4s[currentTri4].m_edge2X[currentZ];
		}

		// normalX
		for (uint32_t currentX = 0u; currentX < m_triangle4s[currentTri4].m_edge2Z.size(); currentX++)
		{
			normalX[currentX] = m_triangle4s[currentTri4].m_edge1Y[currentX] * m_triangle4s[currentTri4].m_edge2Z[currentX] -
				m_triangle4s[currentTri4].m_edge1Z[currentX] * m_triangle4s[currentTri4].m_edge2Y[currentX];
		}

		// normalY
		for (uint32_t currentY = 0u; currentY < m_triangle4s[currentTri4].m_edge2X.size(); currentY++)
		{
			normalY[currentY] = m_triangle4s[currentTri4].m_edge1Z[currentY] * m_triangle4s[currentTri4].m_edge2X[currentY] -
				m_triangle4s[currentTri4].m_edge1X[currentY] * m_triangle4s[currentTri4].m_edge2Z[currentY];
		}

		// normalZ
		for (uint32_t currentZ = 0u; currentZ < m_triangle4s[currentTri4].m_edge2Y.size(); currentZ++)
		{
			normalZ[currentZ] = m_triangle4s[currentTri4].m_edge1X[currentZ] * m_triangle4s[currentTri4].m_edge2Y[currentZ] -
				m_triangle4s[currentTri4].m_edge1Y[currentZ] * m_triangle4s[currentTri4].m_edge2X[currentZ];
		}

		for (uint32_t i = 0u; i < testNormals.size(); i++)
		{
			const Vector3 edge1 = Vector3(m_triangle4s[currentTri4].m_edge1X[i], m_triangle4s[currentTri4].m_edge1Y[i], m_triangle4s[currentTri4].m_edge1Z[i]);
			const Vector3 edge2 = Vector3(m_triangle4s[currentTri4].m_edge2X[i], m_triangle4s[currentTri4].m_edge2Y[i], m_triangle4s[currentTri4].m_edge2Z[i]);

			testNormals[i] = Normalize(Cross(edge1, edge2));
		}

		// Determinants
		for (uint32_t det = 0u; det < pvecX.size(); det++)
		{
			determinants[det] = pvecX[det] * m_triangle4s[currentTri4].m_edge1X[det] +
				pvecY[det] * m_triangle4s[currentTri4].m_edge1Y[det] +
				pvecZ[det] * m_triangle4s[currentTri4].m_edge1Z[det];
		}

		// Calculating ts
		for (uint32_t det = 0u; det < determinants.size(); det++)
		{
			if (std::abs(determinants[det]) >= 1e-8f)
			{
				const float invDet = 1.0f / determinants[det];

				// Calculate tVec
				tvecX[det] = ray.Origin().X() - m_triangle4s[currentTri4].m_v0X[det];
				tvecY[det] = ray.Origin().Y() - m_triangle4s[currentTri4].m_v0Y[det];
				tvecZ[det] = ray.Origin().Z() - m_triangle4s[currentTri4].m_v0Z[det];

				// Calculate u
				u[det] = (tvecX[det] * pvecX[det] +
					tvecY[det] * pvecY[det] +
					tvecZ[det] * pvecZ[det]) * invDet;

				if ((u[det] >= 0.0f) && (u[det] <= 1.0f))
				{
					// Calculating qVec
					qvecX[det] = tvecY[det] * m_triangle4s[currentTri4].m_edge1Z[det] -
						tvecZ[det] * m_triangle4s[currentTri4].m_edge1Y[det];

					qvecY[det] = tvecZ[det] * m_triangle4s[currentTri4].m_edge1X[det] -
						tvecX[det] * m_triangle4s[currentTri4].m_edge1Z[det];

					qvecZ[det] = tvecX[det] * m_triangle4s[currentTri4].m_edge1Y[det] -
						tvecY[det] * m_triangle4s[currentTri4].m_edge1X[det];

					// Calculate v
					v[det] = (ray.Direction().X() * qvecX[det] +
						ray.Direction().Y() * qvecY[det] +
						ray.Direction().Z() * qvecZ[det]) * invDet;

					if ((v[det] >= 0.0f) && ((u[det] + v[det]) <= 1.0f))
					{
						const float potentialT = (m_triangle4s[currentTri4].m_edge2X[det] * qvecX[det] +
							m_triangle4s[currentTri4].m_edge2Y[det] * qvecY[det] +
							m_triangle4s[currentTri4].m_edge2Z[det] * qvecZ[det]) * invDet;

						// Update smallestT at that index
						if ((potentialT >= tMin) && (potentialT <= tMax))
						{
							smallestTs[det] = potentialT;
							tri4Ids[det] = currentTri4;
							potentialNormals[det] = testNormals[det];
							potentialColours[det] = Vector3(1.0f, 0.55f, 0.0f);

							if (T_acceptAnyHit)
							{
								hitFound = true;
								break;
							}
						}
					}
				}

			}
		}
	}

	// Find smallest T
	float t = INFINITY;
	int triId = -1;
	int tri4Id = -1;
	Vector3 normal;
	Vector3 colour;
	for (uint32_t currentT = 0u; currentT < smallestTs.size(); currentT++)
	{
		if (smallestTs[currentT] < t)
		{
			t = smallestTs[currentT];
			triId = currentT;
			tri4Id = tri4Ids[currentT];
			normal = Normalize(potentialNormals[currentT]);
			colour = potentialColours[currentT];
		}
	}

	// Update hit result
	if ((t <= tMax) && (t >= tMin))
	{
		tMax = t;

		out_hitResult.m_t = t;

		out_hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(out_hitResult.m_t);

		out_hitResult.m_colour = colour;

		out_hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;

		out_hitResult.m_primitiveId = (tri4Id * 4u) + triId;
	}
#endif
#ifdef RUNNING_SCALAR_WITH_FACES

	for (uint32_t triangle = 0u; triangle < m_triangles.size(); triangle++)
	{
		const Vector3 edge1 = Vector3(m_triangles[triangle].m_vertices[1u].m_position[0u] - m_triangles[triangle].m_vertices[0u].m_position[0u],
									  m_triangles[triangle].m_vertices[1u].m_position[1u] - m_triangles[triangle].m_vertices[0u].m_position[1u],
									  m_triangles[triangle].m_vertices[1u].m_position[2u] - m_triangles[triangle].m_vertices[0u].m_position[2u]);

		const Vector3 edge2 = Vector3(m_triangles[triangle].m_vertices[2u].m_position[0u] - m_triangles[triangle].m_vertices[0u].m_position[0u],
									  m_triangles[triangle].m_vertices[2u].m_position[1u] - m_triangles[triangle].m_vertices[0u].m_position[1u],
									  m_triangles[triangle].m_vertices[2u].m_position[2u] - m_triangles[triangle].m_vertices[0u].m_position[2u]);

		// Cross product will approach 0s as the directions start facing the same way, or opposite (so parallel)
		const Vector3 pVec = Cross(ray.Direction(), edge2);
		const float det = Dot(pVec, edge1);

		const Vector3 normal = Normalize(Cross(edge1, edge2));

		if (std::fabs(det) >= 1e-8f)
		{
			const float invDet = 1.0f / det;

			const Vector3 tVec = ray.Origin() - Vector3(m_triangles[triangle].m_vertices[0u].m_position[0u], 
														m_triangles[triangle].m_vertices[0u].m_position[1u], 
														m_triangles[triangle].m_vertices[0u].m_position[2u]);

			const float u = Dot(tVec, pVec) * invDet;

			if ((u >= 0.0f) && (u <= 1.0f))
			{
				const Vector3 qVec = Cross(tVec, edge1);
				const float v = Dot(ray.Direction(), qVec) * invDet;

				if ((v >= 0.0f) && ((u + v) <= 1.0f))
				{
					const float t = Dot(edge2, qVec) * invDet;

					if ((t >= tMin) && (t <= tMax))
					{
						tMax = t;

						out_hitResult.m_t = t;

						out_hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(out_hitResult.m_t);

						out_hitResult.m_colour = Vector3(1.0f, 0.55f, 0.0f);

						out_hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;

						out_hitResult.m_primitiveId = triangle;

						if (T_acceptAnyHit)
						{
							break;
						}
					}
				}
			}
		}
	}
#endif
#ifdef RUNNING_SCALAR_WITHOUT_FACES
	// Based on Moller-Trumbore algorithm
	// When you do the SSE, remember it is going to be += 4 and not 3. You'd have to pad in the end, similar to the spheres.
	for (uint32_t triangleOffset = 0u; triangleOffset < m_positionsX.size(); triangleOffset += 3u)
	{
		const Vector3 edge1 = Vector3(m_positionsX[triangleOffset + 1u] - m_positionsX[triangleOffset],
									  m_positionsY[triangleOffset + 1u] - m_positionsY[triangleOffset],
									  m_positionsZ[triangleOffset + 1u] - m_positionsZ[triangleOffset]);

		const Vector3 edge2 = Vector3(m_positionsX[triangleOffset + 2u] - m_positionsX[triangleOffset],
									  m_positionsY[triangleOffset + 2u] - m_positionsY[triangleOffset],
									  m_positionsZ[triangleOffset + 2u] - m_positionsZ[triangleOffset]);

		// Cross product will approach 0s as the directions start facing the same way, or opposite (so parallel)
		const Vector3 pVec = Cross(ray.Direction(), edge2);
		const float det = Dot(pVec, edge1);

		const Vector3 normal = Normalize(Cross(edge1, edge2));

		if (std::fabs(det) >= 1e-8f)
		{
			const float invDet = 1.0f / det;

			const Vector3 tVec = ray.Origin() - Vector3(m_positionsX[triangleOffset], m_positionsY[triangleOffset], m_positionsZ[triangleOffset]);
			const float u = Dot(tVec, pVec) * invDet;

			if ((u >= 0.0f) && (u <= 1.0f))
			{
				const Vector3 qVec = Cross(tVec, edge1);
				const float v = Dot(ray.Direction(), qVec) * invDet;

				if ((v >= 0.0f) && ((u + v) <= 1.0f))
				{
					const float t = Dot(edge2, qVec) * invDet;

					if ((t >= tMin) && (t <= tMax))
					{
						tMax = t;

						out_hitResult.m_t = t;

						out_hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(out_hitResult.m_t);
						
						out_hitResult.m_colour = Vector3(1.0f, 0.55f, 0.0f);

						out_hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;

						out_hitResult.m_primitiveId = triangleOffset / 3u;

						if (T_acceptAnyHit)
						{
							break;
						}
					}
				}
			}
		}
	}
#endif
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
	const HitResult c_primaryHitResult = TraceRay<false>(ray, rayIndex, 1e-5f);
#endif // !TRACE_AGAINST_BVH

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
			const HitResult c_secondaryRayHitResult = TraceRay<true>(c_shadowRay, rayIndex, 1e-5f);
#endif // !TRACE_AGAINST_BVH
			
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
void Renderer::TraverseBVH4(Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult)
{
	bool hasHit = false;
	BVH4DFSTraversal<T_acceptAnyHit>(0u, ray, rayIndex, tMin, tMax, out_hitResult, hasHit);
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
void Renderer::BVH4DFSTraversal(const uint32_t innerNodeStartIndex, Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult, bool& out_hasHit)
{
	ray.m_nodeVisits++;

	const BVH4InnerNode node = m_bvh4AccellStructure->GetInnerNode(innerNodeStartIndex);

	uint32_t hitCount = 0u;
	uint64_t nearPlaneAndChildIndex[4u] = { UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX };
	for (uint32_t child = 0u; child < 4u; child++)
	{
		float tNear = INFINITY;
		if (RayAABBIntersection(ray, node.m_aabbMinX[child], node.m_aabbMinY[child], node.m_aabbMinZ[child], node.m_aabbMaxX[child], node.m_aabbMaxY[child], node.m_aabbMaxZ[child], tMax, &tNear))
		{
			nearPlaneAndChildIndex[hitCount] = (((uint64_t)(*(uint32_t*)(&tNear))) << 32ull) | (uint64_t)child;
			hitCount++;
		}
	}

	// Early out if no hit
	if (!hitCount)
	{
		return;
	}

	// Sort in ascending order
	if (nearPlaneAndChildIndex[0u] > nearPlaneAndChildIndex[1u]) { std::swap(nearPlaneAndChildIndex[0u], nearPlaneAndChildIndex[1u]); }
	if (nearPlaneAndChildIndex[2u] > nearPlaneAndChildIndex[3u]) { std::swap(nearPlaneAndChildIndex[2u], nearPlaneAndChildIndex[3u]); }
	if (nearPlaneAndChildIndex[0u] > nearPlaneAndChildIndex[2u]) { std::swap(nearPlaneAndChildIndex[0u], nearPlaneAndChildIndex[2u]); }
	if (nearPlaneAndChildIndex[1u] > nearPlaneAndChildIndex[3u]) { std::swap(nearPlaneAndChildIndex[1u], nearPlaneAndChildIndex[3u]); }
	if (nearPlaneAndChildIndex[1u] > nearPlaneAndChildIndex[2u]) { std::swap(nearPlaneAndChildIndex[1u], nearPlaneAndChildIndex[2u]); }

#ifdef _DEBUG
	for (uint32_t visitNode = 0u; visitNode < 3u; visitNode++)
	{
		assert(nearPlaneAndChildIndex[visitNode] <= nearPlaneAndChildIndex[visitNode + 1u]);
	}
#endif

	//Traversal
	for (uint32_t child = 0u; child < hitCount; child++)
	{
		const uint32_t visitIndex = (uint32_t)(nearPlaneAndChildIndex[child]);

		if (node.m_child[visitIndex] >> 31u)
		{
			const uint32_t triangleIndex = node.m_child[visitIndex] & ~(1u << 31u);
			HitTriangle<T_acceptAnyHit>(ray, rayIndex, tMin, tMax, triangleIndex, out_hitResult, out_hasHit);
		}
		else if (RayAABBIntersection(ray, node.m_aabbMinX[visitIndex], node.m_aabbMinY[visitIndex], node.m_aabbMinZ[visitIndex], node.m_aabbMaxX[visitIndex], node.m_aabbMaxY[visitIndex], node.m_aabbMaxZ[visitIndex], tMax, nullptr))
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
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
HitResult Renderer::TraceAgainstBVH4(Ray& ray, const uint32_t rayIndex, const float tMin)
{
	HitResult hitResult;
	float tMax = INFINITY;
	TraverseBVH4<T_acceptAnyHit>(ray, rayIndex, tMin, tMax, hitResult);
	return hitResult;
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
void Renderer::TraverseBVH2(Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult)
{
	bool hasHit = false;
	BVH2DFSTraversal<T_acceptAnyHit>(0u, ray, rayIndex, tMin, tMax, out_hitResult, hasHit);
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
void Renderer::BVH2DFSTraversal(const uint32_t innerNodeStartIndex, Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, HitResult& out_hitResult, bool& out_hasHit)
{
	ray.m_nodeVisits++;

	const BVH2InnerNode& node = m_bvh2AccellStructure->GetInnerNode(innerNodeStartIndex);

	float tNears[2u] = { INFINITY, INFINITY };
	float hit[2u] = { false, false };
	hit[0u] = RayAABBIntersection(ray, node.m_leftAABB.m_min[0u]. node.m_leftAABB.m_min[1u], node.m_leftAABB.m_min[2u],
		node.m_leftAABB.m_max[0u], node.m_leftAABB.m_max[1u], node.m_leftAABB.m_max[2u], tMax, &tNears[0u]);
	hit[1u] = RayAABBIntersection(ray, node.m_rightAABB.m_min[0u].node.m_rightAABB.m_min[1u], node.m_rightAABB.m_min[2u],
		node.m_rightAABB.m_max[0u], node.m_rightAABB.m_max[1u], node.m_rightAABB.m_max[2u], tMax, &tNears[0u]);

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
			HitTriangle<T_acceptAnyHit>(ray, rayIndex, tMin, tMax, triangleIndex, out_hitResult, out_hasHit);
		}
		else if (RayAABBIntersection(ray, aabbs[child].m_min[0u], aabbs[child].m_min[1u], aabbs[child].m_min[2u], 
			aabbs[child].m_max[0u], aabbs[child].m_max[1u], aabbs[child].m_max[2u], tMax, nullptr))
		{
			BVH2DFSTraversal<T_acceptAnyHit>(visitOrder[child], ray, rayIndex, tMin, tMax, out_hitResult, out_hasHit);
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
HitResult Renderer::TraceAgainstBVH2(Ray& ray, const uint32_t rayIndex, const float tMin)
{
	HitResult hitResult;
	float tMax = INFINITY;
	TraverseBVH2<T_acceptAnyHit>(ray, rayIndex, tMin, tMax, hitResult);
	return hitResult;
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
void Renderer::HitTriangle(Ray& ray, const uint32_t rayIndex, const float tMin, float& tMax, const uint32_t triangleIndex, HitResult& out_hitResult, bool& out_hasHit)
{
	ray.m_triangleIntersectionTests++;

#ifdef _DEBUG
	assert(rayIndex >= 0u);
#endif
#ifdef NDEBUG
	(void)(rayIndex);
#endif

	const Vector3 edge1 = Vector3(m_triangles[triangleIndex].m_vertices[1u].m_position[0u] - m_triangles[triangleIndex].m_vertices[0u].m_position[0u],
		m_triangles[triangleIndex].m_vertices[1u].m_position[1u] - m_triangles[triangleIndex].m_vertices[0u].m_position[1u],
		m_triangles[triangleIndex].m_vertices[1u].m_position[2u] - m_triangles[triangleIndex].m_vertices[0u].m_position[2u]);

	const Vector3 edge2 = Vector3(m_triangles[triangleIndex].m_vertices[2u].m_position[0u] - m_triangles[triangleIndex].m_vertices[0u].m_position[0u],
		m_triangles[triangleIndex].m_vertices[2u].m_position[1u] - m_triangles[triangleIndex].m_vertices[0u].m_position[1u],
		m_triangles[triangleIndex].m_vertices[2u].m_position[2u] - m_triangles[triangleIndex].m_vertices[0u].m_position[2u]);

	// Cross product will approach 0s as the directions start facing the same way, or opposite (so parallel)
	const Vector3 pVec = Cross(ray.Direction(), edge2);
	const float det = Dot(pVec, edge1);

	const Vector3 normal = Normalize(Cross(edge1, edge2));

	if (std::fabs(det) >= 1e-8f)
	{
		const float invDet = 1.0f / det;

		const Vector3 tVec = ray.Origin() - Vector3(m_triangles[triangleIndex].m_vertices[0u].m_position[0u],
			m_triangles[triangleIndex].m_vertices[0u].m_position[1u],
			m_triangles[triangleIndex].m_vertices[0u].m_position[2u]);

		const float u = Dot(tVec, pVec) * invDet;

		if ((u >= 0.0f) && (u <= 1.0f))
		{
			const Vector3 qVec = Cross(tVec, edge1);
			const float v = Dot(ray.Direction(), qVec) * invDet;

			if ((v >= 0.0f) && ((u + v) <= 1.0f))
			{
				const float t = Dot(edge2, qVec) * invDet;

				if ((t >= tMin) && (t <= tMax))
				{
					tMax = t;

					out_hitResult.m_t = t;

					out_hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(out_hitResult.m_t);

					out_hitResult.m_colour = Vector3(1.0f, 0.55f, 0.0f);

					out_hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;

					out_hitResult.m_primitiveId = triangleIndex;

					if (T_acceptAnyHit)
					{
						out_hasHit = true;
					}
				}
			}
		}
	}
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
HitResult Renderer::TraceRay(Ray& ray, const uint32_t rayIndex, const float tMin)
{
	HitResult hitResult;
	float tMax = INFINITY;
	HitTriangles<T_acceptAnyHit>(ray, rayIndex, tMin, tMax, hitResult);

	return hitResult;
}
