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
};

// --------------------------------------------------------------------------------
class BVHAccellStructure
{
public:

	BVHAccellStructure(const std::vector<Triangle>& triangles, const BVHPartitionStrategy& bvhPartitionStrategy);

	const InnerNode& GetInnerNode(uint32_t index) const;

private:

	ConstructResult ConstructNode(std::vector<Centroid>& centroids, const uint32_t start, 
		const uint32_t end, const BVHPartitionStrategy& bvhPartitionStrategy);
	AABB CalculateAABB(uint32_t triangle);

	std::vector<InnerNode> m_innerNodes;
	std::vector<Triangle> m_triangles;
};

