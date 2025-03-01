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
		centroids[triangle].m_centroid.SetX(0.5f * aabb.m_min.X() + 0.5f * aabb.m_max.X());
		centroids[triangle].m_centroid.SetY(0.5f * aabb.m_min.Y() + 0.5f * aabb.m_max.Y());
		centroids[triangle].m_centroid.SetZ(0.5f * aabb.m_min.Z() + 0.5f * aabb.m_max.Z());
	}

	const ConstructResult cr = ConstructNode(centroids, 0u, (uint32_t)centroids.size() - 1u, bvhPartitionStrategy);
}

// --------------------------------------------------------------------------------
const InnerNode& BVHAccellStructure::GetInnerNode(uint32_t index) const
{
	assert(index < m_innerNodes.size());
	return m_innerNodes[index];
}

// --------------------------------------------------------------------------------
AABB BVHAccellStructure::CalculateAABB(uint32_t triangle)
{
	struct AABB aabb;

	aabb.m_min.SetX(m_triangles[triangle].m_faceVertices[0u].m_position[0u]);
	aabb.m_min.SetY(m_triangles[triangle].m_faceVertices[0u].m_position[1u]);
	aabb.m_min.SetZ(m_triangles[triangle].m_faceVertices[0u].m_position[2u]);

	aabb.m_max.SetX(m_triangles[triangle].m_faceVertices[0u].m_position[0u]);
	aabb.m_max.SetY(m_triangles[triangle].m_faceVertices[0u].m_position[1u]);
	aabb.m_max.SetZ(m_triangles[triangle].m_faceVertices[0u].m_position[2u]);

	for (uint32_t vertex = 1u; vertex < 3u; vertex++)
	{
		const Vector3 currentVertex(m_triangles[triangle].m_faceVertices[vertex].m_position[0u],
			m_triangles[triangle].m_faceVertices[vertex].m_position[1u],
			m_triangles[triangle].m_faceVertices[vertex].m_position[2u]);

		aabb.m_min = Min(aabb.m_min, currentVertex);
		aabb.m_max = Max(aabb.m_max, currentVertex);
	}

	return aabb;
}

// --------------------------------------------------------------------------------
uint32_t ChooseAxisForPartition(const Vector3& min, const Vector3& max)
{
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
// NOTE TO SELF: The aabb computed for the triangle is used for TRAVERSAL, 
// the aabb computed from the centroids is only used to decide on an appropraite split
// the two aabbs computed are not equivalent, spatially.
ConstructResult BVHAccellStructure::ConstructNode(std::vector<Centroid>& centroids, const uint32_t start, 
	const uint32_t end, const BVHPartitionStrategy& bvhPartitionStrategy)
{
	ConstructResult cs;

	if (end == start)
	{
		cs.isLeaf = true;
		cs.m_index = centroids[start].m_triangleIndex;
		cs.m_aabb = CalculateAABB(centroids[start].m_triangleIndex);
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
			Vector3 centroidsMin(centroids[start].m_centroid);
			Vector3 centroidsMax(centroids[start].m_centroid);

			// Calculate the bounding box for the centroids
			for (uint32_t centroidStart = start + 1u; centroidStart < end + 1u; centroidStart++)
			{
				centroidsMin = Min(centroidsMin, centroids[centroidStart].m_centroid);
				centroidsMax = Max(centroidsMax, centroids[centroidStart].m_centroid);
			}

			// Calculate the longest axis
			const uint32_t axis = ChooseAxisForPartition(centroidsMin, centroidsMax);

			const float splitValue = centroidsMin.GetValueByAxisIndex(axis) + ((centroidsMax.GetValueByAxisIndex(axis) - centroidsMin.GetValueByAxisIndex(axis)) / 2.0f);

			auto iterator = std::partition(centroids.begin() + start, centroids.begin() + end + 1, [&](Centroid& centroid) { return centroid.m_centroid.GetValueByAxisIndex(axis) < splitValue; });

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
		case BVHPartitionStrategy::HalfWayLongestAxisWithSAH:
		{
			// TODO: Maybe make a Min/Max for Vector3 to make this easier
			Vector3 centroidsMin(centroids[start].m_centroid);
			Vector3 centroidsMax(centroids[start].m_centroid);

			// Calculate the bounding box for the centroids
			for (uint32_t centroidStart = start + 1u; centroidStart < end + 1u; centroidStart++)
			{
				centroidsMin = Min(centroidsMin, centroids[centroidStart].m_centroid);
				centroidsMax = Max(centroidsMax, centroids[centroidStart].m_centroid);
			}

			// Note: These aabbs are around the actual triangles, not their centroids
			constexpr uint32_t splitCount = 34u;
			constexpr uint32_t splitDivisor = splitCount + 1u;

			float splitValues[3u * splitCount];
			for (uint32_t value = 0u; value < 3u * splitCount; value++)
			{
				splitValues[value] = INFINITY;
			}

			struct AABB aabbs[3u * splitCount * 2u];
			uint32_t triangleCounts[3u * splitCount * 2u] = {};

			// For every triangle
			for (uint32_t centroid = start; centroid < end + 1u; centroid++)
			{
				struct AABB currentTriangleAABB = CalculateAABB(centroids[centroid].m_triangleIndex);

				for (uint32_t axis = 0u; axis < 3u; axis++)
				{
					// I think you can compute the 3 axes lengths outside, and cache them, instead of needlessly recomputing here
					// this is here just for my sanity atm
					const float axisLength = centroidsMax.GetValueByAxisIndex(axis) - centroidsMin.GetValueByAxisIndex(axis);
					const float splitIncrement = axisLength / (float)splitDivisor;

					for (uint32_t split = 0u; split < splitCount; split++)
					{
						const uint32_t splitValueIndex = (axis * splitCount) + split;
						splitValues[splitValueIndex] = centroidsMin.GetValueByAxisIndex(axis) + (((float)split * splitIncrement) + splitIncrement);

						const uint32_t index = (centroids[centroid].m_centroid.GetValueByAxisIndex(axis) < splitValues[splitValueIndex]) ? 
							(axis * splitCount * 2u) + (split * 2u) :
							(axis * splitCount * 2u) + (split * 2u) + 1u;

						aabbs[index].MergeAABB(currentTriangleAABB);
						triangleCounts[index]++;
					}
				}
			}

			// Pick axis to split via SAH
			float bestCost = INFINITY;
			uint32_t bestAxis = UINT32_MAX;
			float bestSplitValue = INFINITY;

			for (uint32_t axis = 0u; axis < 3u; axis++)
			{
				for (uint32_t split = 0u; split < splitCount; split++)
				{
					const uint32_t lIndex = (axis * splitCount * 2u) + (split * 2u);
					const uint32_t rIndex = (axis * splitCount * 2u) + (split * 2u) + 1u;

					struct AABB V;
					V.MergeAABB(aabbs[lIndex]);
					V.MergeAABB(aabbs[rIndex]);

					const float saV = V.GetSurfaceArea();

					float vl = 0.0f;
					float vlSah = 0.0f;
					if (triangleCounts[lIndex] > 0u)
					{
						vl = aabbs[lIndex].GetSurfaceArea();
						vlSah = vl / saV;
					}

					float vr = 0.0f;
					float vrSah = 0.0f;
					if (triangleCounts[rIndex] > 0u)
					{
						vr = aabbs[rIndex].GetSurfaceArea();
						vrSah = vr / saV;
					}

					const float cost = vlSah * (float)triangleCounts[lIndex] +
						vrSah * (float)triangleCounts[rIndex];

					if (cost < bestCost)
					{
						bestCost = cost;
						bestAxis = axis;

						const uint32_t splitValueIndex = (axis * splitCount) + split;
						bestSplitValue = splitValues[splitValueIndex];

						// TO DO:
						// Figure out when a splitValue will ever be set to INFINITY - Split values will never be INFINITY at this stage, they
						// would instead be set to whatever value the increment computed in the loop generating aabbs and splitvalues - this is guarranteed
					}
				}
			}

			// Partitioning
			auto iterator = std::partition(centroids.begin() + start, centroids.begin() + end + 1, [&](Centroid& centroid) { return centroid.m_centroid.GetValueByAxisIndex(bestAxis) < bestSplitValue; });

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
		cs.m_aabb.MergeAABB(left.m_aabb);
		cs.m_aabb.MergeAABB(right.m_aabb);
	}

	return cs;
}
