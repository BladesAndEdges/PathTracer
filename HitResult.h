#pragma once

#include <cmath>
#include <stdint.h>
#include <Vector3.h>

struct HitResult
{
	Vector3 m_intersectionPoint;
	float m_t = INFINITY;
	Vector3 m_colour;
	Vector3 m_normal;
	uint32_t m_primitiveId = UINT32_MAX;
	bool inShadow = false;
	// Figure out if we need padding here
};

