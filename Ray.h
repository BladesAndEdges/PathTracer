#pragma once

#include "Vector3.h"

struct AABB;

class Ray
{
public:
	Ray(const Vector3& origin, const Vector3& direction);

	Vector3 Origin() const;
	Vector3 Direction() const;
	Vector3 CalculateIntersectionPoint(const float t) const;

private:

	Vector3 m_rayOrigin;
	Vector3 m_normalizedRayDir;
};

bool RayAABBIntersection(const Ray& ray, const AABB& aabb);
