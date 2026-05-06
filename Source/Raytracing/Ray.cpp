#include "Ray.h"

#include <xmmintrin.h>
#include <immintrin.h>

#include "BVH2AccellStructure.h"

// --------------------------------------------------------------------------------
Ray::Ray(const Vector3& origin, const Vector3& direction) : m_rayOrigin(origin), m_primaryAABBIntersectionTests(0u), m_primaryTriangleIntersectionTests(0u), m_primaryNodeVisits(0u)
{
	// In case not normalized
	m_normalizedRayDir = Normalize(direction);
	m_inverseDirection = Vector3(1.0f / m_normalizedRayDir.X(), 1.0f / m_normalizedRayDir.Y(), 1.0f / m_normalizedRayDir.Z());
	m_negativeOriginTimesInverseDir = -m_rayOrigin * m_inverseDirection;
}

// --------------------------------------------------------------------------------
Vector3 Ray::CalculateIntersectionPoint(const float t) const
{
	const Vector3 offset = t * m_normalizedRayDir;
	return m_rayOrigin + offset;
}

// --------------------------------------------------------------------------------
// Used from the PBRT book
const float gamma(int n) {
	constexpr float MachineEpsilon = (float)(std::numeric_limits<float>::epsilon() * 0.5);
	return (n * MachineEpsilon) / (1 - n * MachineEpsilon);
}

// --------------------------------------------------------------------------------
// Function implementation follows the PBRT version https://pbr-book.org/4ed/Shapes/Basic_Shape_Interface
// The branchless implementation used was obtained from understanding https://tavianator.com/2015/ray_box_nan.html#footnote-smits
// As per the comemnts in the previous link, some issues were mentioned when dealing with infinitely thin AABBs, but have chosen to ignore this
bool RayAABBIntersection(Ray& ray, bool isPrimary, float minX, float minY, float minZ, float maxX, float maxY, float maxZ, const float tMax, float* out_hitNear)
{
	if (!isPrimary)
	{
		ray.m_primaryAABBIntersectionTests++;
	}

	// Expand x
	const float expTNearX = (minX - ray.Origin().X()) * ray.InverseDirection().X();
	const float expTFarX = (maxX - ray.Origin().X()) * ray.InverseDirection().X();

	const float nearX = std::min(expTNearX, expTFarX);
	float farX = std::max(expTFarX, expTNearX);

	const float t0X = std::max(0.0f, nearX);
	const float t1X = std::min(tMax, farX);

	// Expand Y
	const float expTNearY = (minY - ray.Origin().Y()) * ray.InverseDirection().Y();
	const float expTFarY = (maxY - ray.Origin().Y()) * ray.InverseDirection().Y();

	const float nearY = std::min(expTNearY, expTFarY);
	float farY = std::max(expTFarY, expTNearY);
											
	const float t0Y = std::max(t0X, nearY);
	const float t1Y = std::min(t1X, farY);

	// Expand Z
	const float expTNearZ = (minZ - ray.Origin().Z()) * ray.InverseDirection().Z();
	const float expTFarZ = (maxZ - ray.Origin().Z()) * ray.InverseDirection().Z();

	const float nearZ = std::min(expTNearZ, expTFarZ);
	float farZ = std::max(expTFarZ, expTNearZ);

	const float t0Z = std::max(t0Y, nearZ);
	const float t1Z = std::min(t1Y, farZ);

	const bool expHasHit = (t0Z <= t1Z);

	if (out_hitNear) { *out_hitNear = t0Z; }

	return expHasHit;
}

// --------------------------------------------------------------------------------
int SIMDRayAABBIntersection(Ray& ray, bool isPrimary, const float* minX, const float* minY, const float* minZ, 
	const float* maxX, const float* maxY, const float* maxZ, const float tMax, __m128* out_hitNears)
{
	if (!isPrimary)
	{
		ray.m_primaryAABBIntersectionTests += 4u;
	}

	// 0 and tMAx
	const __m128 zeroReg = _mm_set1_ps(0.0f);
	const __m128 tMaxReg = _mm_set1_ps(tMax);
	
	// AABB paramaters
	const __m128 minXs = _mm_loadu_ps(minX);
	const __m128 minYs = _mm_loadu_ps(minY);
	const __m128 minZs = _mm_loadu_ps(minZ);
	const __m128 maxXs = _mm_loadu_ps(maxX);
	const __m128 maxYs = _mm_loadu_ps(maxY);
	const __m128 maxZs = _mm_loadu_ps(maxZ);
	
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
	const __m128 t1 = _mm_min_ps(_mm_min_ps(exitX, exitY), _mm_min_ps(tMaxReg, exitZ));

	if (out_hitNears)
	{
		*out_hitNears = t0;
	}
	
	// hasIntersected
	const __m128 hasIntersected = _mm_cmpge_ps(t1, t0);
	
	return _mm_movemask_ps(hasIntersected);
}
