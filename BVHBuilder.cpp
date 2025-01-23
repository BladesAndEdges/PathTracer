#include "BVHBuilder.h"

#include <assert.h>
#include <cmath>
#include "BVHNode.h"

// --------------------------------------------------------------------------------
BVHBuilder::BVHBuilder(const std::vector<Face>& triangles) : m_triangles(triangles)
{
	const ConstructResult cs = ConstructNode(0u, (uint32_t)triangles.size());
}

// --------------------------------------------------------------------------------
const TriangleNode& BVHBuilder::GetTriangleNode(uint32_t index) const
{
	assert(index < m_triNodes.size());
	return m_triNodes[index];
}

// --------------------------------------------------------------------------------
const InnerNode& BVHBuilder::GetInnerNode(uint32_t index) const
{
	assert(index < m_innerNodes.size());
	return m_innerNodes[index];
}

// --------------------------------------------------------------------------------
AABB BVHBuilder::CalculateAABB(uint32_t firstTriIndex)
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
ConstructResult BVHBuilder::ConstructNode(uint32_t triangleStartPos, uint32_t triCount)
{
	ConstructResult cs;

	if (triCount == 1u)
	{
		TriangleNode node;
		node.m_index = (uint32_t)m_triNodes.size(); // This works only due to us moving left-to-right along the triangle array

		cs.isLeaf = true;
		cs.m_index = (uint32_t)m_triNodes.size();
		cs.m_aabb = CalculateAABB((uint32_t)m_triNodes.size());

		m_triNodes.push_back(node);
	}
	else
	{ 
		InnerNode node;
		const uint32_t innerNodeIndex = (uint32_t)m_innerNodes.size();
		m_innerNodes.push_back(node);

		const uint32_t leftCount = triCount / 2u;
		const ConstructResult left = ConstructNode(triangleStartPos, leftCount);
		const ConstructResult right = ConstructNode(leftCount, triCount - leftCount);

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
