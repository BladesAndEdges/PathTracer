#include "BVHAccellStructure.h"

#include <assert.h>
#include <cmath>

// --------------------------------------------------------------------------------
BVHAccellStructure::BVHAccellStructure(const std::vector<Triangle>& triangles, const BVHPartitionStrategy& bvhPartitionStrategy) : m_triangles(triangles)
{
	std::vector<Centroid> centroids;
	centroids.resize(triangles.size());

	for (uint32_t triangle = 0u; triangle < triangles.size(); triangle++)
	{
		const AABB aabb = CalculateAABB(triangle);

		centroids[triangle].m_triangleIndex = triangle;
		centroids[triangle].m_centroid[0u] = 0.5f * aabb.min[0u] + 0.5f * aabb.max[0u];
		centroids[triangle].m_centroid[1u] = 0.5f * aabb.min[1u] + 0.5f * aabb.max[1u];
		centroids[triangle].m_centroid[2u] = 0.5f * aabb.min[2u] + 0.5f * aabb.max[2u];
	}

	const ConstructResult cr = ConstructNode(centroids, 0u, (uint32_t)centroids.size() - 1u, bvhPartitionStrategy);
}

// --------------------------------------------------------------------------------
const TriangleNode& BVHAccellStructure::GetTriangleNode(uint32_t index) const
{
	assert(index < m_triangleNodes.size());
	return m_triangleNodes[index];
}

// --------------------------------------------------------------------------------
const InnerNode& BVHAccellStructure::GetInnerNode(uint32_t index) const
{
	assert(index < m_innerNodes.size());
	return m_innerNodes[index];
}

// --------------------------------------------------------------------------------
AABB BVHAccellStructure::CalculateAABB(uint32_t firstTriIndex)
{
	float minX = m_triangles[firstTriIndex].m_faceVertices[0u].m_position[0u];
	float minY = m_triangles[firstTriIndex].m_faceVertices[0u].m_position[1u];
	float minZ = m_triangles[firstTriIndex].m_faceVertices[0u].m_position[2u];

	float maxX = m_triangles[firstTriIndex].m_faceVertices[0u].m_position[0u];
	float maxY = m_triangles[firstTriIndex].m_faceVertices[0u].m_position[1u];
	float maxZ = m_triangles[firstTriIndex].m_faceVertices[0u].m_position[2u];


	for (uint32_t vertex = 1u; vertex < 3u; vertex++)
	{
		if (minX > m_triangles[firstTriIndex].m_faceVertices[vertex].m_position[0u])
		{
			minX = m_triangles[firstTriIndex].m_faceVertices[vertex].m_position[0u];
		}

		if (minY > m_triangles[firstTriIndex].m_faceVertices[vertex].m_position[1u])
		{
			minY = m_triangles[firstTriIndex].m_faceVertices[vertex].m_position[1u];
		}

		if (minZ > m_triangles[firstTriIndex].m_faceVertices[vertex].m_position[2u])
		{
			minZ = m_triangles[firstTriIndex].m_faceVertices[vertex].m_position[2u];
		}

		if (maxX < m_triangles[firstTriIndex].m_faceVertices[vertex].m_position[0u])
		{
			maxX = m_triangles[firstTriIndex].m_faceVertices[vertex].m_position[0u];
		}

		if (maxY < m_triangles[firstTriIndex].m_faceVertices[vertex].m_position[1u])
		{
			maxY = m_triangles[firstTriIndex].m_faceVertices[vertex].m_position[1u];
		}

		if (maxZ < m_triangles[firstTriIndex].m_faceVertices[vertex].m_position[2u])
		{
			maxZ = m_triangles[firstTriIndex].m_faceVertices[vertex].m_position[2u];
		}
	}

	AABB aabb;
	aabb.min[0u] = minX;
	aabb.min[1u] = minY;
	aabb.min[2u] = minZ;
	aabb.max[0u] = maxX;
	aabb.max[1u] = maxY;
	aabb.max[2u] = maxZ;

	return aabb;
}


// --------------------------------------------------------------------------------
AABB MergeAABB(AABB leftAABB, AABB rightAABB)
{
	AABB aabb;
	aabb.min[0u] = std::fmin(leftAABB.min[0u], rightAABB.min[0u]);
	aabb.min[1u] = std::fmin(leftAABB.min[1u], rightAABB.min[1u]);
	aabb.min[2u] = std::fmin(leftAABB.min[2u], rightAABB.min[2u]);

	aabb.max[0u] = std::fmin(leftAABB.max[0u], rightAABB.max[0u]);
	aabb.max[1u] = std::fmin(leftAABB.max[1u], rightAABB.max[1u]);
	aabb.max[2u] = std::fmin(leftAABB.max[2u], rightAABB.max[2u]);

	return aabb;
}

// --------------------------------------------------------------------------------
// NOTE TO SELF: The aabb computed for the triangle is used for TRAVERSAL, 
// the aabb computed from the centroids is only used to decide on an appropraite split
// the two aabbs computed are not equivalent, spatially.
ConstructResult BVHAccellStructure::ConstructNode(std::vector<Centroid>& centroids, const uint32_t start, 
	const uint32_t end, const BVHPartitionStrategy& bvhPartitionStrategy)
{
	ConstructResult cs;

	if (end == start)
	{
		TriangleNode node;
		node.m_index = centroids[start].m_triangleIndex;

		cs.isLeaf = true;
		cs.m_index = centroids[start].m_triangleIndex;
		cs.m_aabb = CalculateAABB(centroids[start].m_triangleIndex);

		m_triangleNodes.push_back(node);
	}
	else
	{ 
		InnerNode node;
		const uint32_t innerNodeIndex = (uint32_t)m_innerNodes.size();
		m_innerNodes.push_back(node);

		uint32_t leftStart, leftEnd;
		uint32_t rightStart, rightEnd;
		switch (bvhPartitionStrategy)
		{
		case BVHPartitionStrategy::HalfWayPoint:
		{
			const uint32_t centroidsInNode = (end - start) + 1u;
			const uint32_t numInLeftSubtree = centroidsInNode / 2u;

			leftStart = start;
			leftEnd = (start + numInLeftSubtree) - 1u;
			rightStart = leftEnd + 1u;
			rightEnd = end;

			break;
		}

		default:
		{
			const uint32_t centroidsInNode = (end - start) + 1u;
			const uint32_t numInLeftSubtree = centroidsInNode / 2u;

			leftStart = start;
			leftEnd = (start + numInLeftSubtree) - 1u;
			rightStart = leftEnd + 1u;
			rightEnd = end;

			break;
		}
		}

		const ConstructResult left = ConstructNode(centroids, leftStart, leftEnd, bvhPartitionStrategy);
		const ConstructResult right = ConstructNode(centroids, rightStart, rightEnd, bvhPartitionStrategy);

		m_innerNodes[innerNodeIndex].m_leftChild = left.m_index;
		m_innerNodes[innerNodeIndex].m_leftAABB = left.m_aabb;
		m_innerNodes[innerNodeIndex].m_leftIsLeaf = left.isLeaf;

		m_innerNodes[innerNodeIndex].m_rightChild = right.m_index;
		m_innerNodes[innerNodeIndex].m_rightAABB = right.m_aabb;
		m_innerNodes[innerNodeIndex].m_rightIsLeaf = right.isLeaf;

		cs.isLeaf = false;
		cs.m_index = innerNodeIndex;
		cs.m_aabb = MergeAABB(left.m_aabb, right.m_aabb);
	}

	return cs;
}
