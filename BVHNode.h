#pragma once

#include <vector>

struct AABB
{
	float min[3u];
	float max[3u];
};

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

struct TriangleNode
{
	uint32_t m_index;
};