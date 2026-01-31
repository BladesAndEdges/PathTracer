#pragma once

#include <vector>

class BVH2AccellStructure;
class BVH4AccellStructure;
struct BVH2InnerNode;
struct BVH4InnerNode;
struct Material4Index;
struct TraversalTriangle;
struct TraversalTriangle4;
struct TriangleIndices;
struct Triangle;
struct TriangleTexCoords;
struct TriangleTexCoords4;
class TriangleAccellStructure;
class Triangle4AccellStructure;

// --------------------------------------------------------------------------------
class TraversalDataManager
{
public:

	TraversalDataManager(const std::vector<Triangle>& triangles, const std::vector<uint32_t> perTriangleMaterials);

	const std::vector<TraversalTriangle>& GetTraversalTriangles() const;
	const std::vector<uint32_t>& GetMaterialIndices() const;
	const std::vector<TriangleTexCoords>& GetTriangleTexCoords() const;

	const std::vector<TraversalTriangle4>& GetTraversalTriangle4s() const;
	const std::vector<Material4Index>& GetMaterial4Indices() const;
	const std::vector<TriangleTexCoords4>& GetTriangleTexCoords4() const;

	const BVH2InnerNode& GetBVH2InnerNode(const uint32_t index) const;
	const TraversalTriangle& GetBVH2TraversalTriangle(const uint32_t index) const;
	const uint32_t GetBVH2MaterialIndex(const uint32_t index) const;
	const TriangleTexCoords& GetBVH2TriangleTexCoords(const uint32_t index) const;

	const BVH4InnerNode& GetBVH4InnerNode(const uint32_t index) const;
	const TraversalTriangle4& GetBVH4TraversalTriangle4(const uint32_t index) const;
	const TriangleIndices& GetBVH4TriangleIndices(const uint32_t index) const;
	const Material4Index& GetBVH4Material4Index(const uint32_t index) const;
	const TriangleTexCoords4& GetBVH4TriangleTexCoords4(const uint32_t index) const;

private:

	TriangleAccellStructure* m_triangleAccellStructure;
	Triangle4AccellStructure* m_triangle4AccellStructure;
	BVH2AccellStructure* m_bvh2AccellStructure;
	BVH4AccellStructure* m_bvh4AccellStructure;
};

