#pragma once

#include "float.h"

struct TriangleTexCoord
{
	float m_v0uv[2u] = { FLT_MAX, FLT_MAX };
	float m_v1uv[2u] = { FLT_MAX, FLT_MAX };
	float m_v2uv[2u] = { FLT_MAX, FLT_MAX };
};

