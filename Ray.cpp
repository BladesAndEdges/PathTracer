#include "Ray.h"

#include "BVH2AccellStructure.h"

// --------------------------------------------------------------------------------
Ray::Ray(const Vector3& origin, const Vector3& direction) : m_rayOrigin(origin), m_aabbIntersectionTests(0u), m_triangleIntersectionTests(0u), m_nodeVisits(0u)
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
bool RayAABBIntersection(Ray& ray, float minX, float minY, float minZ, float maxX, float maxY, float maxZ, const float tMax, float* out_hitNear)
{
	ray.m_aabbIntersectionTests++;

	// MADD test   ( a   *    b)                       +    c
	float tNearX = (minX * ray.InverseDirection().X()) + ray.NegativeOriginTimesInvDir().X();
	float tFarX = (maxX * ray.InverseDirection().X()) + ray.NegativeOriginTimesInvDir().X();
	
	// y axis
	float tNearY = (minY * ray.InverseDirection().Y()) + ray.NegativeOriginTimesInvDir().Y();
	float tFarY = (maxY * ray.InverseDirection().Y()) + ray.NegativeOriginTimesInvDir().Y();
	
	// z axis
	float tNearZ = (minZ * ray.InverseDirection().Z()) + ray.NegativeOriginTimesInvDir().Z();
	float tFarZ = (maxZ * ray.InverseDirection().Z()) + ray.NegativeOriginTimesInvDir().Z();

	const float t0 = std::max(std::max(std::max(std::min(tNearX, tFarX), std::min(tNearY, tFarY)), std::min(tNearZ, tFarZ)), 0.0f);
	const float t1 = std::min(std::min(std::min(std::max(tNearX, tFarX), std::max(tNearY, tFarY)), std::max(tNearZ, tFarZ)), tMax);
	
	if (out_hitNear) { *out_hitNear = t0; }
	
	return t1 >= t0;
}
