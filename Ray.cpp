#include "Ray.h"

#include "BVHAccellStructure.h"

// --------------------------------------------------------------------------------
Ray::Ray(const Vector3& origin, const Vector3& direction) : m_rayOrigin(origin), m_aabbIntersectionTests(0u), m_triangleIntersectionTests(0u)
{
	// In case not normalized
	m_normalizedRayDir = Normalize(direction);
	m_inverseDirection = Vector3(1.0f / m_normalizedRayDir.X(), 1.0f / m_normalizedRayDir.Y(), 1.0f / m_normalizedRayDir.Z());
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
bool RayAABBIntersection(Ray& ray, const struct AABB& aabb)
{
	ray.m_aabbIntersectionTests++;

	float t0 = 0.0f;
	float t1 = INFINITY;

	for (uint32_t axis = 0u; axis < 3u; axis++)
	{
		float tNear = (aabb.m_min.GetValueByAxisIndex(axis) - ray.Origin().GetValueByAxisIndex(axis)) * ray.InverseDirection().GetValueByAxisIndex(axis);
		float tFar = (aabb.m_max.GetValueByAxisIndex(axis) - ray.Origin().GetValueByAxisIndex(axis)) * ray.InverseDirection().GetValueByAxisIndex(axis);

		// Handle ray traveling in negative axis direction
		if (tNear > tFar) { std::swap(tNear, tFar); }

		// Pad the results in order to avoid NaNs
		tFar *= 1 + 2 * gamma(3);

		//Update t0 and t1, based on the tigher bounds after planes clip the ray
		t0 = (tNear > t0) ? tNear : t0;
		t1 = (tFar < t1) ? tFar : t1;

		// If no overlap detected
		if (t0 > t1) { return false; }
	}
	
	return true;
}


