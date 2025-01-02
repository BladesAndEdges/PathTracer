#pragma once

#include <cmath>
#include <stdint.h>
#include <Vector3.h>

struct HitResult
{
	Vector3 m_intersectionPoint = Vector3(0.0f, 0.0f, 0.0f);
	float m_t = INFINITY;
	Vector3 m_colour = Vector3(0.98f, 0.28f, 0.89f);
	Vector3 m_normal = Vector3(-1.0f, -1.0f, -1.0f);
	uint32_t m_primitiveId = UINT32_MAX;
};

