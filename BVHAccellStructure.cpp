#include "BVHAccellStructure.h"

#include <assert.h>
#include <algorithm>
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
		centroids[triangle].m_centroid = 0.5f * aabb.m_min + 0.5f * aabb.m_max;
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
	aabb.m_min = Vector3(minX, minY, minZ);
	aabb.m_max = Vector3(maxX, maxY, maxZ);

	return aabb;
}


// --------------------------------------------------------------------------------
AABB MergeAABB(AABB leftAABB, AABB rightAABB)
{
	AABB aabb;
	aabb.m_min = Vector3(std::fmin(leftAABB.m_min.X(), rightAABB.m_min.X()),
		std::fmin(leftAABB.m_min.Y(), rightAABB.m_min.Y()),
		std::fmin(leftAABB.m_min.Z(), rightAABB.m_min.Z()));

	aabb.m_max = Vector3(std::fmax(leftAABB.m_max.X(), rightAABB.m_max.X()),
		std::fmax(leftAABB.m_max.Y(), rightAABB.m_max.Y()),
		std::fmax(leftAABB.m_max.Z(), rightAABB.m_max.Z()));

	return aabb;
}

// --------------------------------------------------------------------------------
uint32_t ChooseAxisForPartition(const Vector3& min, const Vector3& max)
{
	assert(max.X() > min.X());

	const Vector3 length = max - min;

	if ((length.Y() > length.X()) && (length.Y() > length.Z()))
	{
			return 1u;
	}

	if ((length.Z() > length.X()) && (length.Z() > length.Y()))
	{
			return 2u;
	}

	return 0u;
}

// --------------------------------------------------------------------------------
float CalculateSplitPointNonSAH(const Vector3& min, const Vector3& max, const uint32_t axis)
{
	float splitPoint = FP_INFINITE;
	switch (axis)
	{
	case 0:
		splitPoint = max.X() + ((max.X() - min.X()) / 2.0f);
		break;
	case 1:
		splitPoint = max.Y() + ((max.Y() - min.Y()) / 2.0f);
		break;
	case 2:
		splitPoint = max.Y() + ((max.Z() - min.Z()) / 2.0f);
		break;
	default:
		__debugbreak();
		break;
	}

	return splitPoint;
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
			Vector3 min = centroids[start].m_centroid;
			Vector3 max = centroids[start].m_centroid;

			// Calculate the bounding box for the centroids
			for (uint32_t centroid = start + 1u; centroid < end + 1u; centroid++)
			{
				min = Min(min, centroids[centroid].m_centroid);
				max = Max(max, centroids[centroid].m_centroid);
			}

			const uint32_t axis = ChooseAxisForPartition(min, max);
			const float splitPoint = CalculateSplitPointNonSAH(min, max, axis);

			auto iterator = std::partition(centroids.begin() + start, centroids.begin() + end + 1, [&](Centroid& centroid) { return centroid.GetCentroidValueByAxis(axis) < splitPoint; });

			const uint32_t indexOfRightNode = (uint32_t)std::distance(centroids.begin(), iterator);

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

// --------------------------------------------------------------------------------
float Centroid::GetCentroidValueByAxis(const uint32_t axis) const
{
	float value = FP_INFINITE;
	switch (axis)
	{
	case 0u:
		value = m_centroid.X();
		break;
	case 1u:
		value = m_centroid.Y();
		break;
	case 2u:
		value = m_centroid.Z();
		break;
	default:
		__debugbreak();
		break;
	}

	return value;
}
