#pragma once

#include <vector>

#include "AABB.h"
#include "Vector3.h"

struct ConstructResult
{
	uint32_t m_index;
	AABB m_aabb;
	bool isLeaf;
};

struct InnerNode
{
	uint32_t m_leftChild;
	uint32_t m_rightChild;
	AABB m_leftAABB;
	AABB m_rightAABB;
	bool m_leftIsLeaf;
	bool m_rightIsLeaf;
};