#include "Ray.h"

#include "BVHAccellStructure.h"

// --------------------------------------------------------------------------------
Ray::Ray(const Vector3& origin, const Vector3& direction) : m_rayOrigin(origin)
{
	// In case not normalized
	m_normalizedRayDir = Normalize(direction);
}

// --------------------------------------------------------------------------------
Vector3 Ray::Origin() const
{
	return m_rayOrigin;
}

// --------------------------------------------------------------------------------
Vector3 Ray::Direction() const
{
	return m_normalizedRayDir;
}

// --------------------------------------------------------------------------------
Vector3 Ray::CalculateIntersectionPoint(const float t) const
{
	const Vector3 offset = t * m_normalizedRayDir;
	return m_rayOrigin + offset;
}

// --------------------------------------------------------------------------------
const float gamma(int n) {
	constexpr float MachineEpsilon = (float)(std::numeric_limits<float>::epsilon() * 0.5);
	return (n * MachineEpsilon) / (1 - n * MachineEpsilon);
}

// --------------------------------------------------------------------------------
// Function implementation follows the PBRT version https://pbr-book.org/4ed/Shapes/Basic_Shape_Interface
bool RayAABBIntersection(const Ray& ray, const AABB& aabb)
{
	float t0 = 0.0f;
	float t1 = INFINITY;

	const float rayOrigin[3u] = { ray.Origin().X(), ray.Origin().Y(), ray.Origin().Z()};
	const float rayDirection[3u] = {ray.Direction().X(), ray.Direction().Y(), ray.Direction().Z()};

	for (uint32_t ax = 0u; ax < 3u; ax++)
	{
		const float invDir = 1.0f / rayDirection[ax];
		
		// Find tNear and tFar values for the slab for the axis
		float tNear = (aabb.min[ax] - rayOrigin[ax]) * invDir;
		float tFar = (aabb.max[ax] - rayOrigin[ax]) * invDir;

		// Handle ray traveling in negative axis direction
		if (tNear > tFar) { std::swap(tNear, tFar); }

		// Pad the results in order to avoid NaNs
		tFar *= 1 + 2 * gamma(3);

		//Update t0 and t1, based on the tigher bounds after planes clip the ray
		t0 = (tNear > t0) ? tNear : t0;
		t1 = (tFar < t1) ? tFar : t1;
	}

	return true;
}


