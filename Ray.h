#pragma once

#include "Vector3.h"

class Ray
{
public:

	Ray(const Vector3& origin, const Vector3& direction);

private:

	Vector3 m_rayOrigin;
	Vector3 m_normalizedRayDir;
};

