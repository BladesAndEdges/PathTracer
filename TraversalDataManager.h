#pragma once

#include <vector>

class BVH2AccellStructure;
struct BVH2InnerNode;
struct TraversalTriangle;
struct TraversalTriangle4;
struct Triangle;
class TriangleAccellStructure;
class Triangle4AccellStructure;

// --------------------------------------------------------------------------------
class TraversalDataManager
{
public:

	TraversalDataManager(const std::vector<Triangle>& triangles);

	const std::vector<TraversalTriangle>& GetTraversalTriangles() const;
	const std::vector<TraversalTriangle4>& GetTraversalTriangle4s() const;
	const BVH2InnerNode& GetBVH2InnerNode(const uint32_t index) const;
	const TraversalTriangle& GetBVH2TraversalTriangle(const uint32_t index) const;

private:

	TriangleAccellStructure* m_triangleAccellStructure;
	Triangle4AccellStructure* m_triangle4AccellStructure;
	BVH2AccellStructure* m_bvh2AccellStructure;
};

