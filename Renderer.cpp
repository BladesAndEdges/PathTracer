#include "Renderer.h"

#include <assert.h>
#include <emmintrin.h>
#include <xmmintrin.h>

#include "Framebuffer.h"
#include "PerformanceCounter.h"
#include "Vector3.h"

# define M_PI 3.14159265358979323846
//# define RUNNING_SCALAR
#define RUNNING_SCALAR_WITHOUT_FACES

// --------------------------------------------------------------------------------
Renderer::Renderer(const std::vector<float>& positionsX, const std::vector<float>& positionsY, const std::vector<float>& positionsZ,
	const std::vector<Triangle4>& triangle4s, const std::vector<Face>& faces, const Vector3& center)	
																										: m_positionsX(positionsX), 
																										  m_positionsY(positionsY),
																										  m_positionsZ(positionsZ),
																										  m_triangle4s(triangle4s),
																										  m_faces(faces),
																										  m_center(center)
{
	// Once planes are multiple, this would be changed
	c_indigo.SetX(1.0f);
	c_indigo.SetY(0.0f);
	c_indigo.SetZ(0.0f);

	c_white.SetX(1.0f);
	c_white.SetY(1.0f);
	c_white.SetZ(1.0f);

	c_grey.SetX(0.75f);
	c_grey.SetY(0.75f);
	c_grey.SetZ(0.75f);

	Vector3 c_green;
	c_green.SetX(0.0f);
	c_green.SetY(1.0f);
	c_green.SetZ(0.0f);

	Vector3 c_red;
	c_red.SetX(1.0f);
	c_red.SetY(0.0f);
	c_red.SetZ(0.0f);

	Vector3 c_black;
	c_black.SetX(0.0f);
	c_black.SetY(0.0f);
	c_black.SetZ(0.0f);

	// Generate spheres
	const float c_startingPosX = 0.0f;
	const float c_startingPosZ = -1.0f;

#ifdef RUNNING_SCALAR
	for (int i = 0; i < 10u; i++)
	{
		for (int j = 0; j < 10u; j++)
		{
			const Vector3 c_position(c_startingPosX + (float)i, 0.4f, c_startingPosZ - (float)j);
			m_sphereList.push_back(c_position);

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
#endif // RUNNING_SCALAR


#ifndef RUNNING_SCALAR
	//for (int i = 0; i < 5; i++)
	//{
	//	for (int j = 0; j < 5; j++)
	//	{
	//		// Radii and positional data
	//		m_sphereRadii.push_back(0.4f);
	//		m_sphereCentersX.push_back(c_startingPosX + (float)i);
	//		m_sphereCentersY.push_back(0.4f);
	//		m_sphereCentersZ.push_back(c_startingPosZ - (float)j);
	//
	//		if ((i % 2) == 0)
	//		{
	//			m_sphereColours.push_back(c_green);
	//		}
	//		else
	//		{
	//			m_sphereColours.push_back(c_blue);
	//		}
	//	}
	//}

	m_sphereRadii.push_back(0.7f);
	m_sphereCentersX.push_back(c_startingPosX);
	m_sphereCentersY.push_back(0.7f);
	m_sphereCentersZ.push_back(c_startingPosZ);
	m_sphereColours.push_back(c_red);

	//m_sphereRadii.push_back(2.0);
	//m_sphereCentersX.push_back(c_startingPosX + 0.1f);
	//m_sphereCentersY.push_back(2.0f);
	//m_sphereCentersZ.push_back(c_startingPosZ);
	//m_sphereColours.push_back(c_white);

	m_sphereRadii.push_back(0.7f);
	m_sphereCentersX.push_back(c_startingPosX  + (2.0f * 0.7f) + 0.1f);
	m_sphereCentersY.push_back(0.7f);
	m_sphereCentersZ.push_back(c_startingPosZ);
	m_sphereColours.push_back(c_green);

	// Check if the "SOA" should be padded
	if ((m_sphereCentersX.size() % 4) != 0u)
	{
		const uint32_t c_numToPad = 4 - m_sphereCentersX.size() % 4;

		for (uint32_t padding = 0u; padding < c_numToPad; padding++)
		{
			m_sphereRadii.push_back(0.0f);
			m_sphereCentersX.push_back(0.0f);
			m_sphereCentersY.push_back(0.0f);
			m_sphereCentersZ.push_back(0.0f);
			m_sphereColours.push_back(c_black);
		}
	}

	// Assert all are padded correctly
	assert((m_sphereRadii.size() % 4u) == 0u);
	assert((m_sphereCentersX.size() % 4u) == 0u);
	assert((m_sphereCentersY.size() % 4u) == 0u);
	assert((m_sphereCentersZ.size() % 4u) == 0u);

	// All arrays match in size
	assert(m_sphereRadii.size() == m_sphereCentersX.size());
	assert(m_sphereRadii.size() == m_sphereCentersY.size());
	assert(m_sphereRadii.size() == m_sphereCentersZ.size());
#endif

	const float quadSize = 4.0f;
	const Vector3 quadPivot(-2.0f, 1.0f, -2.0f);

	// Back
	m_quadList.push_back(quadPivot);
	m_quadUs.push_back(Vector3(quadSize, 0.0f, 0.0f));
	m_quadVs.push_back(Vector3(0.0f, quadSize, 0.0f));
	m_quadColours.push_back(Vector3(1.0f, 1.0f, 0.0f));
	
	// Left
	m_quadList.push_back(quadPivot);
	m_quadUs.push_back(Vector3(0.0f, 0.0f, quadSize));
	m_quadVs.push_back(Vector3(0.0f, quadSize, 0.0f));
	m_quadColours.push_back(Vector3(1.0f, 0.0f, 0.0f));

	// Right
	m_quadList.push_back(quadPivot + Vector3(quadSize, 0.0f, 0.0f));
	m_quadUs.push_back(Vector3(0.0f, 0.0f, quadSize));
	m_quadVs.push_back(Vector3(0.0f, quadSize, 0.0f));
	m_quadColours.push_back(Vector3(0.0f, 0.0f, 1.0f));
	
	// Top
	m_quadList.push_back(quadPivot + Vector3(0.0f, quadSize, 0.0f));
	m_quadUs.push_back(Vector3(quadSize, 0.0f, 0.0f));
	m_quadVs.push_back(Vector3(0.0f, 0.0f, quadSize));
	m_quadColours.push_back(Vector3(1.0f, 0.7f, 0.4f));
	
	// Bottom
	m_quadList.push_back(quadPivot);
	m_quadUs.push_back(Vector3(quadSize, 0.0f, 0.0f));
	m_quadVs.push_back(Vector3(0.0f, 0.0f, quadSize));
	m_quadColours.push_back(Vector3(0.0f, 1.0f, 0.0f));
	
	// Position if flipping the z axis
	m_center.SetZ(m_center.Z() + 12.0f);
	m_camera.SetCameraLocation(m_center);
	m_lightDirection = Normalize(Vector3(1.0f, 1.0f, 1.0f));

	ZeroMemory((void*)&m_viewportDesc, sizeof(m_viewportDesc));
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
		RegenerateViewSpaceDirections(framebuffer);
	}

	//----------------------------------------------------------------------------------------------------------------------------------------------------------------
	uint8_t* bytes = framebuffer->GetDataPtr();
	for (uint32_t row = 0u; row < framebuffer->GetHeight(); row++)
	{
		for (uint32_t column = 0u; column < framebuffer->GetWidth(); column++)
		{
			const uint32_t rayIndex = (row * framebuffer->GetWidth() + column);

			// Byte offsets
			const uint32_t texelByteIndex = (row * framebuffer->GetWidth() * framebuffer->GetNumChannels()) + (column * framebuffer->GetNumChannels());
			assert(texelByteIndex < (framebuffer->GetWidth() * framebuffer->GetHeight() * framebuffer->GetNumChannels()));

			Vector3 radiance(0.0f, 0.0f, 0.0f);
			const uint32_t numSamples = 1u;
			const uint32_t depth = 2u;
			for (uint32_t sample = 0u; sample < numSamples; sample++)
			{
				Vector3 texelTopLeft;
				Vector3 texelBottomRight;
				
				texelTopLeft.SetX(m_texelCenters[rayIndex].X() - m_viewportDesc.m_texelWidth / 2.0f);
				texelTopLeft.SetY(m_texelCenters[rayIndex].Y() + m_viewportDesc.m_texelHeight / 2.0f);
				
				texelBottomRight.SetX(m_texelCenters[rayIndex].X() + m_viewportDesc.m_texelWidth / 2.0f);
				texelBottomRight.SetY(m_texelCenters[rayIndex].Y() - m_viewportDesc.m_texelHeight / 2.0f);
				
				const float randomX = RandomFloat(texelTopLeft.X(), texelBottomRight.X());
				const float randomY = RandomFloat(texelBottomRight.Y(), texelTopLeft.Y());

				const Ray c_primaryRay(m_camera.GetCameraLocation(), Vector3(randomX, randomY, -1.0f));

				radiance = radiance + PathTrace(c_primaryRay, depth);
			}

				radiance = Vector3(radiance.X() / (float)numSamples, radiance.Y() / (float)numSamples, radiance.Z() / (float)numSamples);

				const float red = std::fmin(1.0f, radiance.X());
				const float green = std::fmin(1.0f, radiance.Y());
				const float blue = std::fmin(1.0f, radiance.Z());

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
			texelCenter.SetX(texelCenter.X() + (m_viewportDesc.m_texelWidth / 2.0f));
			texelCenter.SetY(texelCenter.Y() - (m_viewportDesc.m_texelHeight / 2.0f));
			m_texelCenters.push_back(texelCenter);
		}
	}

	m_isFirstFrame = false;
}

// --------------------------------------------------------------------------------
template<bool T_acceptAnyHit>
void Renderer::HitSphere(const Ray& ray, const float tMin, float& tMax, HitResult& out_hitResult)
{

#ifdef RUNNING_SCALAR
	//========
	const float sphereRadiusSquared = 0.4f * 0.4f;
	const float a = Dot(ray.Direction(), ray.Direction());
	const float fourTimesA = 4.0f * a;
	const float denom = 1.0f / (2.0f * a);
	const Vector3 doubleRayDir = -2.0f * ray.Direction();
	
	int sphereId = -1;
	for (uint32_t sphere = 0u; sphere < m_sphereList.size(); sphere++)
	{
		const Vector3 rayOriginToSphere = m_sphereList[sphere] - ray.Origin();
		const float b = Dot(doubleRayDir, rayOriginToSphere);
		const float c = Dot(rayOriginToSphere, rayOriginToSphere) - sphereRadiusSquared;
	
		// If an intersection has occurred
		const float discriminant = b * b - fourTimesA * c;
		if (discriminant >= 0.0f)
		{
			// If the intersection is closer than previously stored distance
			const float t = (-b - sqrtf(discriminant)) * denom;
			if (t >= tMin && t <= tMax)
			{
				tMax = t;
				
				out_hitResult.m_t = t;
	
				sphereId = sphere;
	
				if (T_acceptAnyHit) { break; }
			}
		}
	}

	if (sphereId != -1)
	{
		out_hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(out_hitResult.m_t);
		out_hitResult.m_colour = m_sphereColours[sphereId];
		const Vector3 sphereCenter = m_sphereList[sphereId];
		out_hitResult.m_normal = Normalize(out_hitResult.m_intersectionPoint - m_sphereList[sphereId]);
	}

#endif
#ifndef RUNNING_SCALAR
	//========
	const __m128 tMinimum = _mm_set1_ps(tMin);
	const __m128 tMaximum = _mm_set1_ps(tMax);

	// Ray data
	const __m128 rayOriginX = _mm_set1_ps(ray.Origin().X());
	const __m128 rayOriginY = _mm_set1_ps(ray.Origin().Y());
	const __m128 rayOriginZ = _mm_set1_ps(ray.Origin().Z());

	const __m128 c_rayDirectionX = _mm_set1_ps(ray.Direction().X());
	const __m128 c_rayDirectionY = _mm_set1_ps(ray.Direction().Y());
	const __m128 c_rayDirectionZ = _mm_set1_ps(ray.Direction().Z());

	// Calculate a = Dot(ray.Direction(), ray.Direction())
	const __m128 c_adx = _mm_mul_ps(c_rayDirectionX, c_rayDirectionX);
	const __m128 c_ady = _mm_mul_ps(c_rayDirectionY, c_rayDirectionY);
	const __m128 c_adz = _mm_mul_ps(c_rayDirectionZ, c_rayDirectionZ);

	const __m128 c_aa0 = _mm_add_ps(c_adx, c_ady);
	const __m128 a = _mm_add_ps(c_aa0, c_adz);

	//Calculate 4.0f * a
	const __m128 c_allFours = _mm_set1_ps(4.0f);
	const __m128 c_fourTimesA = _mm_mul_ps(c_allFours, a);

	// Calculate -2.0f * ray.Direction()
	const __m128 c_allNegativeTwos = _mm_set1_ps(-2.0f);
	const __m128 c_invDoubleRayDirectionX = _mm_mul_ps(c_rayDirectionX, c_allNegativeTwos);
	const __m128 c_invDoubleRayDirectionY = _mm_mul_ps(c_rayDirectionY, c_allNegativeTwos);
	const __m128 c_invDoubleRayDirectionZ = _mm_mul_ps(c_rayDirectionZ, c_allNegativeTwos);

	// Calculate denominator. Multiply both parts of the fraction by two. 1/2a becomes 2/4a, and reuse 4a
	const __m128 c_allTwos = _mm_set1_ps(2.0f);
	const __m128 c_denom = _mm_div_ps(c_allTwos, c_fourTimesA);

	// "Be careful where ray origin = sphereCente

	// Will be updated in the loop
	__m128 smallestTs = _mm_set1_ps(INFINITY); //Initialize to tMaximum for one less comparison in the loop (?)
	__m128i smallestTIndices = _mm_set1_epi32(-1);

	// Loop variables
	const __m128i c_incrementRegister = _mm_set1_epi32(4);
	__m128i currentSphereIndices = _mm_set_epi32(3, 2, 1, 0);

	for (uint32_t sphere = 0u; sphere < m_sphereRadii.size(); sphere+=4u)
	{
		const __m128 c_sphereRadii = _mm_loadu_ps(m_sphereRadii.data() + sphere);
		const __m128 c_sphereRadiiSquared = _mm_mul_ps(c_sphereRadii, c_sphereRadii);

		const __m128 c_sphereCentersX = _mm_loadu_ps(m_sphereCentersX.data() + sphere);
		const __m128 c_sphereCentersY = _mm_loadu_ps(m_sphereCentersY.data() + sphere);
		const __m128 c_sphereCentersZ = _mm_loadu_ps(m_sphereCentersZ.data() + sphere);

		// RayOrigin-to-sphere vector
		const __m128 rayOriginToSphereX = _mm_sub_ps(c_sphereCentersX, rayOriginX);
		const __m128 rayOriginToSphereY = _mm_sub_ps(c_sphereCentersY, rayOriginY);
		const __m128 rayOriginToSphereZ = _mm_sub_ps(c_sphereCentersZ, rayOriginZ);

		// Calculate b = Dot(invDoubleRayDir, rayOriginToSphere)
		const __m128 c_bdx = _mm_mul_ps(c_invDoubleRayDirectionX, rayOriginToSphereX); // fmadd
		const __m128 c_bdy = _mm_mul_ps(c_invDoubleRayDirectionY, rayOriginToSphereY);
		const __m128 c_bdz = _mm_mul_ps(c_invDoubleRayDirectionZ, rayOriginToSphereZ);

		const __m128 c_ba0 = _mm_add_ps(c_bdx, c_bdy);
		const __m128 b = _mm_add_ps(c_ba0, c_bdz);

		// Calculate Dot(RayOriginToSphere, RayOriginToSphere)
		const __m128 c_cdx = _mm_mul_ps(rayOriginToSphereX, rayOriginToSphereX);
		const __m128 c_cdy = _mm_mul_ps(rayOriginToSphereY, rayOriginToSphereY);
		const __m128 c_cdz = _mm_mul_ps(rayOriginToSphereZ, rayOriginToSphereZ);

		const __m128 c_ca0 = _mm_add_ps(c_cdx, c_cdy);
		const __m128 c_ca1 = _mm_add_ps(c_ca0, c_cdz);

		// Calculate c = Dot(RayOriginToSphere, RayOriginToSphere) - sphereRadiusSquared
		const __m128 c = _mm_sub_ps(c_ca1, c_sphereRadiiSquared);

		// Calculate discriminant
		const __m128 c_bSquared = _mm_mul_ps(b, b);
		const __m128 c_discriminantRhs = _mm_mul_ps(c_fourTimesA, c);
		const __m128 discriminant = _mm_sub_ps(c_bSquared, c_discriminantRhs);

		// Check if it intersects a sphere
		const __m128 c_allZeros = _mm_set1_ps(0.0f);
		const __m128 hasIntersectedMask = _mm_cmpge_ps(discriminant, c_allZeros);

		// Calculate t = (-b - sqrtf(discriminant)) * denom
		const __m128 negativeB = _mm_sub_ps(_mm_set1_ps(0.0f), b);
		const __m128 sqrtDiscriminant = _mm_sqrt_ps(discriminant);
		const __m128 tLhs = _mm_sub_ps(negativeB, sqrtDiscriminant);
		const __m128 t = _mm_mul_ps(tLhs, c_denom);

		const __m128 tMinimumMask = _mm_cmpge_ps(t, tMinimum);
		const __m128 tMaximumMask = _mm_cmple_ps(t, tMaximum);

		// Bitwise and all the masks. Intersection, less than tMin and greater than tMax
		const __m128 validTValuesMask = _mm_and_ps(hasIntersectedMask, _mm_and_ps(tMinimumMask, tMaximumMask));

		// Get the valid ts that can then be tested against the currently stored smallest ts
		const __m128 potentialTs = _mm_or_ps(_mm_and_ps(validTValuesMask, t), _mm_andnot_ps(validTValuesMask, tMaximum));

		// Get the indices that will be kept, and the indices that should be changed
		const __m128i c_lessThanMask = _mm_castps_si128(_mm_cmplt_ps(potentialTs, smallestTs));
		const __m128i c_oldIndicesToKeep = _mm_andnot_si128(c_lessThanMask, smallestTIndices);
		const __m128i c_newIndicesToUpdate = _mm_and_si128(c_lessThanMask, currentSphereIndices);

		// Update values
		smallestTIndices = _mm_or_si128(c_oldIndicesToKeep, c_newIndicesToUpdate);
		smallestTs = _mm_min_ps(smallestTs, potentialTs);

		if (T_acceptAnyHit)
		{
			if (_mm_movemask_ps(validTValuesMask)) 
			{ 
				break; 
			}
		}

		// Update indices for next iteration
		currentSphereIndices = _mm_add_epi32(currentSphereIndices, c_incrementRegister);
	}

	// Find the smallest t in the lanes
	// Figure out a better way to name this
	const __m128 firstMinimumShuffle = _mm_shuffle_ps(smallestTs, smallestTs, _MM_SHUFFLE(0, 0, 2, 3));
	const __m128 firstMinimumComparison = _mm_min_ps(smallestTs, firstMinimumShuffle);

	const __m128i firstIndexShuffle = _mm_shuffle_epi32(smallestTIndices, _MM_SHUFFLE(0, 0, 2, 3));
	const __m128i firstIndexMask = _mm_castps_si128(_mm_cmplt_ps(firstMinimumShuffle, smallestTs));
	const __m128i firstBlahBlah = _mm_or_si128(_mm_and_si128(firstIndexMask, firstIndexShuffle), _mm_andnot_si128(firstIndexMask, smallestTIndices));

	// Compare into the 0th lane
	const __m128 secondMinimumShuffle = _mm_shuffle_ps(firstMinimumComparison, firstMinimumComparison, _MM_SHUFFLE(0, 0, 0, 1));
	const __m128 secondComparison = _mm_min_ps(firstMinimumComparison, secondMinimumShuffle);

	const __m128i secondIndexShuffle = _mm_shuffle_epi32(firstBlahBlah, _MM_SHUFFLE(0, 0, 0, 1));
	const __m128i secondIndexMask = _mm_castps_si128(_mm_cmplt_ps(secondMinimumShuffle, firstMinimumComparison));
	const __m128i secondBlahBlah = _mm_or_si128(_mm_and_si128(secondIndexMask, secondIndexShuffle), _mm_andnot_si128(secondIndexMask, firstBlahBlah));

	tMax = _mm_cvtss_f32(secondComparison);
	out_hitResult.m_t = _mm_cvtss_f32(secondComparison);
	const int sphereId = _mm_cvtsi128_si32(secondBlahBlah);

	if (sphereId > -1)
	{
		out_hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(out_hitResult.m_t);
		out_hitResult.m_colour = m_sphereColours[sphereId];

		const Vector3 sphereCenter(m_sphereCentersX[sphereId], m_sphereCentersY[sphereId], m_sphereCentersZ[sphereId]);

		const Vector3 c_tempNormal = Normalize(out_hitResult.m_intersectionPoint - sphereCenter);

		// Front facing and back facing normals
		if (Dot(ray.Direction(), c_tempNormal) > 0.0f)
		{
			out_hitResult.m_normal = -c_tempNormal;
		}
		else
		{
			out_hitResult.m_normal = c_tempNormal;
		}
	}
#endif
}

// --------------------------------------------------------------------------------
void Renderer::HitPlane(const Ray& ray, const float tMin, float& tMax, const float distance, const Vector3& normal, Vector3 colour, HitResult& out_hitResult)
{
	const float denom = Dot(normal, ray.Direction());

	if (std::fabs(denom) >= 1e-8f)
	{
		const float t = (distance - Dot(normal, ray.Origin())) / denom;

		if (t >= tMin && t <= tMax)
		{
			tMax = t;

			out_hitResult.m_t = t;

			out_hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(out_hitResult.m_t);
			out_hitResult.m_colour = colour;
			out_hitResult.m_normal = normal;
		}
	}
}

// --------------------------------------------------------------------------------
void Renderer::HitQuad(const Ray& ray, const float tMin, float& tMax, HitResult& out_hitResult)
{
	// If u and v are unit of length, no need to normalize
	// Back, Left, Right, Top, Bottom
	for (uint32_t quad = 0u; quad < m_quadList.size(); quad++)
	{
		const Vector3 Q = m_quadList[quad];
		const Vector3 u = m_quadUs[quad];
		const Vector3 v = m_quadVs[quad];

		const Vector3 n = Cross(u, v);
		Vector3 normal = Normalize(n);

		const float normDenom = Dot(n, n);
		const Vector3 w = Vector3(n.X() / normDenom, n.Y() / normDenom, n.Z() / normDenom);

		const float distance = Dot(normal, Q);

		const float denom = Dot(normal, ray.Direction());

		if (std::fabs(denom) >= 1e-8f)
		{
			const float t = (distance - Dot(normal, ray.Origin())) / denom;

			if (t >= tMin && t <= tMax)
			{
				// Determine if the hit point lies within the planar shape using its plane coordinates.
				const Vector3 intersectionPoint = ray.CalculateIntersectionPoint(t);

				const Vector3 planarHitpVector = intersectionPoint - Q;

				const float alpha = Dot(w, Cross(planarHitpVector, v));
				const float beta = Dot(w, Cross(u, planarHitpVector));

				if (IsInsideQuad(alpha, beta))
				{
					tMax = t;

					out_hitResult.m_t = t;

					out_hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(out_hitResult.m_t);
					out_hitResult.m_colour = m_quadColours[quad];
					out_hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;
				}
			}
		}
	}
}

// --------------------------------------------------------------------------------
bool Renderer::IsInsideQuad(const float alpha, const float beta)
{
	return (0.0f <= alpha) && (alpha <= 1.0f) && (0.0f <= beta) && (beta <= 1.0f);
}

// --------------------------------------------------------------------------------
void Renderer::HitTriangle(const Ray& ray, const float tMin, float& tMax, HitResult& out_hitResult)
{
#ifndef RUNNING_SSE_TRI4
	//for(uint32_t triangle = 0; triangle <)
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

		const Vector3 normal = Cross(edge1, edge2);

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

					if (t >= tMin && t <= tMax)
					{
						tMax = t;

						out_hitResult.m_t = t;

						out_hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(out_hitResult.m_t);
						out_hitResult.m_colour = Vector3(1.0f, 0.55f, 0.0f);
						out_hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;
					}
				}
			}
		}
	}
#endif
#if RUNNING_SCALAR_WITH_FACES
	//-------------------------------------------------------------------------------------------------------------------------
	// Based on Moller-Trumbore algorithm
	for (uint32_t face = 0u; face < m_faces.size(); face++)
	{
		const Vector3 edge1 = Vector3(m_faces[face].m_faceVertices[1u].m_position[0u] - m_faces[face].m_faceVertices[0u].m_position[0u],
			m_faces[face].m_faceVertices[1u].m_position[1u] - m_faces[face].m_faceVertices[0u].m_position[1u], 
			m_faces[face].m_faceVertices[1u].m_position[2u] - m_faces[face].m_faceVertices[0u].m_position[2u]); // v0v1

		const Vector3 edge2 = Vector3(m_faces[face].m_faceVertices[2u].m_position[0u] - m_faces[face].m_faceVertices[0u].m_position[0u],
			m_faces[face].m_faceVertices[2u].m_position[1u] - m_faces[face].m_faceVertices[0u].m_position[1u],
			m_faces[face].m_faceVertices[2u].m_position[2u] - m_faces[face].m_faceVertices[0u].m_position[2u]); // v0v2
	
		// Cross product will approach 0s as the directions start facing the same way, or opposite (so parallel)
		const Vector3 pVec = Cross(ray.Direction(), edge2);
		const float det = Dot(pVec, edge1);
	
		const Vector3 normal = Cross(edge1, edge2);
	
		if (std::fabs(det) >= 1e-8f)
		{
			const float invDet = 1.0f / det;
	
			const Vector3 tVec = ray.Origin() - Vector3(m_faces[face].m_faceVertices[0u].m_position[0u], 
														m_faces[face].m_faceVertices[0u].m_position[1u], 
														m_faces[face].m_faceVertices[0u].m_position[2u]);

			const float u = Dot(tVec, pVec) * invDet;
	
			if ((u >= 0.0f) && (u <= 1.0f))
			{
				const Vector3 qVec = Cross(tVec, edge1);
				const float v = Dot(ray.Direction(), qVec) * invDet;
	
				if ((v >= 0.0f) && ((u + v) <= 1.0f))
				{
					const float t = Dot(edge2, qVec) * invDet;
	
					if (t >= tMin && t <= tMax)
					{
						tMax = t;
	
						out_hitResult.m_t = t;
	
						out_hitResult.m_intersectionPoint = ray.CalculateIntersectionPoint(out_hitResult.m_t);
						out_hitResult.m_colour = Vector3(1.0f, 0.55f, 0.0f);
						out_hitResult.m_normal = (Dot(normal, ray.Direction()) < 0.0f) ? normal : -normal;
					}
				}
			}
		}
	}
#endif
}

// --------------------------------------------------------------------------------
Vector3 Renderer::PathTrace(const Ray& ray, uint32_t depth)
{
	if (depth <= 0u)
	{
		return Vector3(0.0f, 0.0f, 0.0f);
	}

	Vector3 radiance(0.0f, 0.0f, 0.0f);
	const HitResult c_primaryHitResult = TraceRay<false>(ray, 1e-5f);
	if (c_primaryHitResult.m_t != INFINITY)
	{
		// Indirect lighting
		{
			// Calculate the random direction of the outward ray
			const Ray rayOnHemisphere = Ray(c_primaryHitResult.m_intersectionPoint, Vector3::RandomVector3OnHemisphere(c_primaryHitResult.m_normal));

			// RENDERING EQUATION
			
			// We need the Li
			const Vector3 Li = PathTrace(rayOnHemisphere, depth - 1u);
			
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
			
			const Ray c_shadowRay(c_primaryHitResult.m_intersectionPoint, m_lightDirection);
			const HitResult c_secondaryRayHitResult = TraceRay<true>(c_shadowRay, 1e-5f);
			
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
HitResult Renderer::TraceRay(const Ray& ray, const float tMin)
{
	HitResult hitResult;
	float tMax = INFINITY;
	//HitSphere<T_acceptAnyHit>(ray, tMin, tMax, hitResult);
	//HitQuad(ray, tMin, tMax, hitResult);
	HitTriangle(ray, tMin, tMax, hitResult);
	HitPlane(ray, tMin, tMax, 0.0f, Normalize(Vector3(0.0f, 1.0f, 0.0f)), c_grey, hitResult);

	return hitResult;
}
