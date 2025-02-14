#include "BVHAccellStructure.h"

#include <assert.h>
#include <algorithm>
#include <cmath>

// --------------------------------------------------------------------------------
BVHAccellStructure::BVHAccellStructure(const std::vector<Triangle>& triangles, const BVHPartitionStrategy& bvhPartitionStrategy) : m_triangles(triangles)
{
	// Debug triangles because cornell's numbers are annoying as hell to work with


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

	aabb.max[0u] = std::fmax(leftAABB.max[0u], rightAABB.max[0u]);
	aabb.max[1u] = std::fmax(leftAABB.max[1u], rightAABB.max[1u]);
	aabb.max[2u] = std::fmax(leftAABB.max[2u], rightAABB.max[2u]);

	return aabb;
}

// --------------------------------------------------------------------------------
uint32_t ChooseAxisForPartition(const float minX, const float maxX, const float minY, const float maxY, const float minZ, const float maxZ)
{
	const float xLength = maxX - minX;
	const float yLength = maxY - minY;
	const float zLength = maxZ - minZ;

	if ((yLength > xLength) && (yLength > zLength))
	{
			return 1u;
	}

	if ((zLength > xLength) && (zLength > yLength))
	{
			return 2u;
	}

	return 0u;
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
		case BVHPartitionStrategy::HalfWayLongestAxis:
		{
			// TODO: Maybe make a Min/Max for Vector3 to make this easier
			float centroidMin[3u] = { centroids[start].m_centroid[0u], centroids[start].m_centroid[1u] , centroids[start].m_centroid[2u] };
			float centroidMax[3u] = { centroids[start].m_centroid[0u], centroids[start].m_centroid[1u] , centroids[start].m_centroid[2u] };

			// Calculate the bounding box for the centroids
			for (uint32_t centroidStart = start + 1u; centroidStart < end + 1u; centroidStart++)
			{
				if (centroidMin[0u] > centroids[centroidStart].m_centroid[0u])
				{
					centroidMin[0u] = centroids[centroidStart].m_centroid[0u];
				}

				if (centroidMax[0u] < centroids[centroidStart].m_centroid[0u])
				{
					centroidMax[0u] = centroids[centroidStart].m_centroid[0u];
				}

				if (centroidMin[1u] > centroids[centroidStart].m_centroid[1u])
				{
					centroidMin[1u] = centroids[centroidStart].m_centroid[1u];
				}

				if (centroidMax[1u] < centroids[centroidStart].m_centroid[1u])
				{
					centroidMax[1u] = centroids[centroidStart].m_centroid[1u];
				}

				if (centroidMin[2u] > centroids[centroidStart].m_centroid[2u])
				{
					centroidMin[2u] = centroids[centroidStart].m_centroid[2u];
				}

				if (centroidMax[2u] < centroids[centroidStart].m_centroid[2u])
				{
					centroidMax[2u] = centroids[centroidStart].m_centroid[2u];
				}
			}

			// Calculate the longest axis
			const uint32_t axis = ChooseAxisForPartition(centroidMin[0u], centroidMax[0u], centroidMin[1u], centroidMax[1u], centroidMin[2u], centroidMax[2u]);

			const float splitPoint = centroidMin[axis] + ((centroidMax[axis] - centroidMin[axis]) / 2.0f);

			auto iterator = std::partition(centroids.begin() + start, centroids.begin() + end + 1, [&](Centroid& centroid) { return centroid.m_centroid[axis] < splitPoint; });

			const uint32_t indexOfRightNode = (uint32_t)std::distance(centroids.begin(), iterator);

			// The issue is by the time we have two nodes, due to the values, 
			// we calculate a min/max that is the same point. So finding a splitPoint returns the min/max value
			// Now in the partition section, the issue becomes that both nodes end up at the right node
			// So for that iteration we get leftStart, leftEnd, rightStart and rightEnd all equal to 0 on the below lines
			// This ends up continuing infinitely after that, I believe (have not checked it as it is late) ...

			if ((end - start) == 1u)
			{
				leftStart = start;
				leftEnd = start;
				rightStart = end;
				rightEnd = end;
			}
			else
			{
				leftStart = start;
				leftEnd = (indexOfRightNode == 0u) ? indexOfRightNode : indexOfRightNode - 1u;
				rightStart = indexOfRightNode;
				rightEnd = end;

				// Recalculate if all nodes end up on a single child
				// Use halfway split on the nodes
				if ((leftEnd == end) || (rightStart == start))
				{
					const uint32_t centroidsInNode = (end - start) + 1u;
					const uint32_t numInLeftSubtree = centroidsInNode / 2u;

					leftStart = start;
					leftEnd = (start + numInLeftSubtree) - 1u;
					rightStart = leftEnd + 1u;
					rightEnd = end;
				}
			}

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
