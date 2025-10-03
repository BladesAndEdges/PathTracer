#pragma once

#include <vector>

#include "BVHNode.h"
#include "Triangle.h"

// --------------------------------------------------------------------------------
// This idea is taken from pbrt, 4th edition
enum class BVH2PartitionStrategy
{
	HalfWayPoint, 
	HalfWayLongestAxis, 
	HalfWayLongestAxisWithSAH
};

// --------------------------------------------------------------------------------
struct Centroid
{
	Vector3 m_position;
	uint32_t m_triangleIndex;
};

// Triangle's centroid and the aabb
// --------------------------------------------------------------------------------
struct BVHTriangleData
{
	Centroid m_centroid;
	AABB m_aabb;
};

// --------------------------------------------------------------------------------
class BVH2AccellStructure
{
public:

	BVH2AccellStructure(const std::vector<Triangle>& triangles, const BVH2PartitionStrategy& bvhPartitionStrategy);

	const BVH2InnerNode GetInnerNode(uint32_t index) const;
	const Triangle GetTriangle(uint32_t index) const;

	uint32_t GetNodeCount() const;

private:

	ConstructResult ConstructNode(std::vector<BVHTriangleData>& bvhPartitionData, const uint32_t start,
		const uint32_t end, const BVH2PartitionStrategy& bvhPartitionStrategy);
	ConstructResult ConstructNode(BVHTriangleData* bvhData, const uint32_t count, const BVH2PartitionStrategy& bvhPartitionStrategy);
	AABB CalculateAABB(uint32_t triangle);

	std::vector<BVH2InnerNode> m_innerNodes;
	std::vector<Triangle> m_triangles;
};

