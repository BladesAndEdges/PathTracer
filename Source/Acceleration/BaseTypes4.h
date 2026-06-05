#pragma once

#include <float.h>
#include <stdint.h>

// --------------------------------------------------------------------------------
struct MaterialIndex4
{
	uint32_t m_index[4u] = { UINT32_MAX, UINT32_MAX,
						UINT32_MAX, UINT32_MAX };
};

// --------------------------------------------------------------------------------
struct TriangleIndex4
{
	uint32_t m_index[4u] = { UINT32_MAX, UINT32_MAX,
							UINT32_MAX, UINT32_MAX };
};

// --------------------------------------------------------------------------------
struct TriangleTexCoord4
{
	float m_v0U[4u] = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
	float m_v0V[4u] = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };

	float m_v1U[4u] = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
	float m_v1V[4u] = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };

	float m_v2U[4u] = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
	float m_v2V[4u] = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
};