#pragma once

#include <stdint.h>

// --------------------------------------------------------------------------------
struct alignas(16) BVH4Node
{
	float m_aabbMinX[4u];
	float m_aabbMinY[4u];
	float m_aabbMinZ[4u];

	float m_aabbMaxX[4u];
	float m_aabbMaxY[4u];
	float m_aabbMaxZ[4u];

	uint32_t m_child[4u];

	int32_t m_validity[4u];
};