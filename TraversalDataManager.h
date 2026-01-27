#pragma once

#include <vector>

class BVH2AccellStructure;
class BVH4AccellStructure;
struct BVH2InnerNode;
struct BVH4InnerNode;
struct TraversalTriangle;
struct TraversalTriangle4;
struct TriangleIndices;
struct Triangle;
class TriangleAccellStructure;
class Triangle4AccellStructure;

// --------------------------------------------------------------------------------
class TraversalDataManager
{
public:

	TraversalDataManager(const std::vector<Triangle>& triangles, const std::vector<uint32_t> perTriangleMaterials);

	const std::vector<TraversalTriangle>& GetTraversalTriangles() const;
	const std::vector<TraversalTriangle4>& GetTraversalTriangle4s() const;
	const BVH2InnerNode& GetBVH2InnerNode(const uint32_t index) const;
	const TraversalTriangle& GetBVH2TraversalTriangle(const uint32_t index) const;
	const BVH4InnerNode& GetBVH4InnerNode(const uint32_t index) const;
	const TraversalTriangle4& GetBVH4TraversalTriangle4(const uint32_t index) const;
	const TriangleIndices& GetBVH4TriangleIndices(const uint32_t index) const;

private:

	TriangleAccellStructure* m_triangleAccellStructure;
	Triangle4AccellStructure* m_triangle4AccellStructure;
	BVH2AccellStructure* m_bvh2AccellStructure;
	BVH4AccellStructure* m_bvh4AccellStructure;
};

