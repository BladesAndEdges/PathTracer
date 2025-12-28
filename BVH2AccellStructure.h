#pragma once

#include <vector>

#include "BVHNode.h"

struct BVH2InnerNode;
struct ConstructResult;
struct TraversalTriangle;
struct Triangle;

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

	BVH2AccellStructure(const std::vector<Triangle>& triangles, const std::vector<TraversalTriangle>& traversalTriangles, const BVH2PartitionStrategy& bvhPartitionStrategy);

	const BVH2InnerNode& GetInnerNode(uint32_t index) const;
	const TraversalTriangle& GetTraversalTriangle(const uint32_t index) const;

	uint32_t GetNodeCount() const;

private:

	ConstructResult ConstructNode(std::vector<BVHTriangleData>& bvhPartitionData, const uint32_t start,
		const uint32_t end, const BVH2PartitionStrategy& bvhPartitionStrategy);
	ConstructResult ConstructNode(BVHTriangleData* bvhData, const uint32_t count, const BVH2PartitionStrategy& bvhPartitionStrategy);
	AABB CalculateAABB(const uint32_t triangle);

	std::vector<BVH2InnerNode> m_innerNodes;
	std::vector<TraversalTriangle> m_traversalTriangles;
};

