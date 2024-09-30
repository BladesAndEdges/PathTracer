#pragma once

#include <cmath>
#include "RGB.h"
#include <Vector3.h>

struct HitResult
{
	Vector3 m_intersectionPoint;
	float m_t = INFINITY;
	RGB m_colour;
	Vector3 m_normal;
	// Figure out if we need padding here
};

