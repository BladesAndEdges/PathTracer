#pragma once

#include <vector>

#include "AABB.h"
#include "Vector3.h"

struct ConstructResult
{
	uint32_t m_index;
	AABB m_aabb;
};

struct alignas(64) InnerNode
{
	uint32_t m_leftChild;
	uint32_t m_rightChild;
	AABB m_leftAABB;
	AABB m_rightAABB;
	uint32_t padding0;
	uint32_t padding1;
};