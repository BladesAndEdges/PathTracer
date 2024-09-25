#include "Ray.h"

// --------------------------------------------------------------------------------
Ray::Ray(const Vector3& origin, const Vector3& direction) : m_rayOrigin(origin)
{
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
