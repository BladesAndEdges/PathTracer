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

	BVH4AccellStructure(const BVH2AccellStructure* bvhAccellStructure);

	ConstructResult ConstructBVH4Node(const BVH2AccellStructure* bvhAccellStructure, const uint32_t grandParentIndex);


private:

	std::vector<BVH4InnerNode> m_bvh4InnerNodes;
};

     