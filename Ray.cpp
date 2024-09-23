#include "Ray.h"

// --------------------------------------------------------------------------------
Ray::Ray(const Vector3& origin, const Vector3& direction) : m_rayOrigin(origin)
{
	m_normalizedRayDir = Normalize(direction);
}
