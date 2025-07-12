#pragma once

#include <stdint.h>
#include "Vector3.h"

struct AABB;

class Ray
{
public:
	Ray(const Vector3& origin, const Vector3& direction);

	inline Vector3 Origin() const;
	inline Vector3 Direction() const;
	inline Vector3 InverseDirection() const;
	inline Vector3 NegativeOriginTimesInvDir() const;
	Vector3 CalculateIntersectionPoint(const float t) const;

	uint32_t m_aabbIntersectionTests;
	uint32_t m_triangleIntersectionTests;
	uint32_t m_nodeVisits;

private:

	Vector3 m_rayOrigin;
	Vector3 m_normalizedRayDir;
	Vector3 m_inverseDirection;
	Vector3 m_negativeOriginTimesInverseDir;
};

bool RayAABBIntersection(Ray& ray, float minX, float minY, float minZ, float maxX, float maxY, float maxZ, const float tMax, float* out_hitNear);

#include "Ray.inl"
