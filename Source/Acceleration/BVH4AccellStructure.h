#pragma once

#include <stdint.h>
#include <vector>

class BVH2AccellStructure;
struct BVH4Node;
struct MaterialIndex4;
struct TraversalTriangle4;
struct TriangleIndex4;
struct TriangleTexCoord4;

// --------------------------------------------------------------------------------
class BVH4AccellStructure
{
public:

	BVH4AccellStructure(const BVH2AccellStructure* bvh2AccellStructure);
	uint32_t MakeBVH4Node(const BVH2AccellStructure* bvhAccellStructure, const uint32_t start);

	const BVH4Node& GetBVH4Node(const uint32_t index) const;
	const MaterialIndex4& GetMaterialIndex4(const uint32_t index) const;
	const TraversalTriangle4& GetTraversalTriangle4(const uint32_t index) const;
	const TriangleIndex4& GetTriangleIndex4(const uint32_t index) const;
	const TriangleTexCoord4& GetTriangleTexCoord4(const uint32_t index) const;

private:

	std::vector<BVH4Node> m_bvh4Nodes;
	std::vector<MaterialIndex4> m_materialIndex4s;
	std::vector<TraversalTriangle4> m_traversalTriangle4s;
	std::vector<TriangleIndex4> m_triangleIndex4s;
	std::vector<TriangleTexCoord4> m_triangleTexCoord4s;
};

     