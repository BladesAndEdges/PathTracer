#pragma once

#include <vector>

#include "BVHNode.h"
#include "Triangle.h"

// --------------------------------------------------------------------------------
// This idea is taken from pbrt, 4th edition
enum class BVHPartitionStrategy
{
	HalfWayPoint, 
	HalfWayLongestAxis, 
	HalfWayLongestAxisWithSAH
};

// --------------------------------------------------------------------------------
struct Centroid
{
	Vector3 m_centroid;
	uint32_t m_triangleIndex;

	float GetCentroidValueByAxis(const uint32_t axis) const;
};

// --------------------------------------------------------------------------------
class BVHAccellStructure
{
public:

	BVHAccellStructure(const std::vector<Triangle>& triangles, const BVHPartitionStrategy& bvhPartitionStrategy);

	const TriangleNode& GetTriangleNode(uint32_t index) const;
	const InnerNode& GetInnerNode(uint32_t index) const;

private:

	ConstructResult ConstructNode(std::vector<Centroid>& centroids, const uint32_t start, 
		const uint32_t end, const BVHPartitionStrategy& bvhPartitionStrategy);
	AABB CalculateAABB(uint32_t firstTriIndex);

	std::vector<TriangleNode> m_triangleNodes;
	std::vector<InnerNode> m_innerNodes;
	std::vector<Triangle> m_triangles;
};

