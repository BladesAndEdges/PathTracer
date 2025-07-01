#pragma once

#include "BVHNode.h"

class BVH2AccellStructure;

struct BVH4InnerNode
{
	AABB m_bbox[4u];
	uint32_t m_child[4u];
};

class BVH4AccellStructure
{
public:

	BVH4AccellStructure(const BVH2AccellStructure* bvh2AccellStructure);
	uint32_t BuildBVH4NodeFromBVH2Node(const BVH2AccellStructure * bvhAccellStructure, const uint32_t start);

	const BVH4InnerNode GetInnerNode(const uint32_t index) const;
	const uint32_t GetNumInnderNodes() const;

private:

	std::vector<BVH4InnerNode> m_innerNodes;
};

     