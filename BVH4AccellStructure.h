#pragma once

#include <stdint.h>
#include <vector>

class BVH2AccellStructure;
struct TraversalTriangle4;

// --------------------------------------------------------------------------------
struct alignas(16) BVH4InnerNode
{
	float m_aabbMinX[4u];
	float m_aabbMinY[4u];
	float m_aabbMinZ[4u];

	float m_aabbMaxX[4u];
	float m_aabbMaxY[4u];
	float m_aabbMaxZ[4u];

	uint32_t m_child[4u];
};

// --------------------------------------------------------------------------------
class BVH4AccellStructure
{
public:

	BVH4AccellStructure(const BVH2AccellStructure* bvh2AccellStructure);
	uint32_t BuildBVH4NodeFromBVH2NodeTri4(const BVH2AccellStructure * bvhAccellStructure, const uint32_t start);

	const BVH4InnerNode& GetInnerNodeTri4(const uint32_t index) const;
	const TraversalTriangle4& GetTraversalTriangle4(const uint32_t index) const;

private:

	std::vector<BVH4InnerNode> m_innerNodesTri4;
	std::vector<TraversalTriangle4> m_traversalTriangle4s;
};

     