#include "Ray.h"

#include <algorithm>

// --------------------------------------------------------------------------------
Ray::Ray(const Vector3& origin, const Vector3& direction) : m_rayOrigin(origin)
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
