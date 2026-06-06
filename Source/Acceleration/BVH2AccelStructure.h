#pragma once

#include <vector>

#include "AABB.h"
#include "Vector3.h"

struct BVH2Node;
struct Triangle;
struct TraversalTriangle;
struct TriangleTexCoord;

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

// --------------------------------------------------------------------------------
struct BVHTriangleData
{
	Centroid m_centroid;
	AABB m_aabb;
};

// --------------------------------------------------------------------------------
struct ConstructResult
{
	uint32_t m_index;
	AABB m_aabb;
};

// --------------------------------------------------------------------------------
class BVH2AccelStructure
{
public:

	BVH2AccelStructure(const std::vector<Triangle>& triangles, const std::vector<TraversalTriangle>& traversalTriangles, 
		const std::vector<uint32_t>& triangleMaterials, const std::vector<TriangleTexCoord>& triangleTexCoords, const BVH2PartitionStrategy& bvhPartitionStrategy);

	const BVH2Node& GetBVH2Node(uint32_t index) const;
	const TraversalTriangle& GetTraversalTriangle(const uint32_t index) const;
	uint32_t GetMaterialIndex(const uint32_t index) const;
	const TriangleTexCoord& GetTriangleTexCoord(const uint32_t index) const;

	uint32_t GetNodeCount() const;

private:

	ConstructResult ConstructNode(std::vector<BVHTriangleData>& bvhPartitionData, const uint32_t start,
		const uint32_t end, const BVH2PartitionStrategy& bvhPartitionStrategy);
	ConstructResult ConstructNode(BVHTriangleData* bvhData, const uint32_t count, const BVH2PartitionStrategy& bvhPartitionStrategy);
	AABB CalculateAABB(const uint32_t triangle);

	std::vector<BVH2Node> m_bvh2Nodes;
	std::vector<TraversalTriangle> m_traversalTriangles;
	std::vector<uint32_t> m_materialIndices;
	std::vector<TriangleTexCoord> m_triangleTexCoords;
};

