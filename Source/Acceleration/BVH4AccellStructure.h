#pragma once

#include <stdint.h>
#include <vector>

class BVH2AccellStructure;
struct Material4Index;
struct TraversalTriangle4;
struct TriangleTexCoords4;

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
struct TriangleIndices
{
	uint32_t m_triangleIndices[4u] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
};

// --------------------------------------------------------------------------------
class BVH4AccellStructure
{
public:

	BVH4AccellStructure(const BVH2AccellStructure* bvh2AccellStructure);
	uint32_t BuildBVH4NodeFromBVH2NodeTri4(const BVH2AccellStructure * bvhAccellStructure, const uint32_t start);

	const BVH4InnerNode& GetInnerNodeTri4(const uint32_t index) const;
	const TraversalTriangle4& GetTraversalTriangle4(const uint32_t index) const;
	const TriangleIndices& GetTriangleIndices(const uint32_t index) const;
	const Material4Index& GetMaterial4Index(const uint32_t index) const;
	const TriangleTexCoords4& GetTriangleTexCoords4(const uint32_t index) const;

private:

	std::vector<BVH4InnerNode> m_innerNodesTri4;
	std::vector<TraversalTriangle4> m_traversalTriangle4s;
	std::vector<TriangleIndices> m_triangleIndices;
	std::vector<Material4Index> m_material4Indices;
	std::vector<TriangleTexCoords4> m_triangleTexCoords4;
};

     