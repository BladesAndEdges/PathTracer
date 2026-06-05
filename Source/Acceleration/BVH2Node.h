#pragma once

#include "AABB.h"

// --------------------------------------------------------------------------------
struct alignas(64) BVH2Node
{
	BVH2Node();

	uint32_t m_leftChild;
	uint32_t m_rightChild;
	AABB m_leftAABB;
	AABB m_rightAABB;
	uint32_t padding0;
	uint32_t padding1;
};