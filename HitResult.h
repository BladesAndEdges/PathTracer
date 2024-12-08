#pragma once

#include <cmath>
#include <Vector3.h>

struct HitResult
{
	Vector3 m_intersectionPoint;
	float m_t = INFINITY;
	Vector3 m_colour;
	Vector3 m_normal;
	// Figure out if we need padding here
};

