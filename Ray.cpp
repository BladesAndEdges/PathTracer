#include "Ray.h"

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
