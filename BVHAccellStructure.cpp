#include "BVHAccellStructure.h"

#include <assert.h>
#include <algorithm>
#include <cmath>

// --------------------------------------------------------------------------------
BVHAccellStructure::BVHAccellStructure(const std::vector<Triangle>& triangles, const BVHPartitionStrategy& bvhPartitionStrategy) : m_triangles(triangles)
{
	std::vector<BVHPartitionData> bvhPartitionData;
	bvhPartitionData.resize(triangles.size());
	
	for (uint32_t triangle = 0u; triangle < triangles.size(); triangle++)
	{
		bvhPartitionData[triangle].m_aabb = CalculateAABB(triangle);
		
		bvhPartitionData[triangle].m_centroid.m_triangleIndex = triangle;
		bvhPartitionData[triangle].m_centroid.m_position.SetX(0.5f * bvhPartitionData[triangle].m_aabb.m_min.X() + 0.5f * bvhPartitionData[triangle].m_aabb.m_max.X());
		bvhPartitionData[triangle].m_centroid.m_position.SetY(0.5f * bvhPartitionData[triangle].m_aabb.m_min.Y() + 0.5f * bvhPartitionData[triangle].m_aabb.m_max.Y());
		bvhPartitionData[triangle].m_centroid.m_position.SetZ(0.5f * bvhPartitionData[triangle].m_aabb.m_min.Z() + 0.5f * bvhPartitionData[triangle].m_aabb.m_max.Z());
	}
	
	const ConstructResult cr = ConstructNode(bvhPartitionData, 0u, (uint32_t)bvhPartitionData.size() - 1u, bvhPartitionStrategy);
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
	AABB aabb;

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
ConstructResult BVHAccellStructure::ConstructNode(std::vector<BVHPartitionData>& bvhPartitionData, const uint32_t start, 
	const uint32_t end, const BVHPartitionStrategy& bvhPartitionStrategy)
{
	ConstructResult cs;

	if (end == start)
	{
		cs.m_index = bvhPartitionData[start].m_centroid.m_triangleIndex | (1u << 31u);
		cs.m_aabb = bvhPartitionData[start].m_aabb;
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
			Vector3 centroidsMin(bvhPartitionData[start].m_centroid.m_position);
			Vector3 centroidsMax(bvhPartitionData[start].m_centroid.m_position);

			// Calculate the bounding box for the centroids
			for (uint32_t centroidStart = start + 1u; centroidStart < end + 1u; centroidStart++)
			{
				centroidsMin = Min(centroidsMin, bvhPartitionData[centroidStart].m_centroid.m_position);
				centroidsMax = Max(centroidsMax, bvhPartitionData[centroidStart].m_centroid.m_position);
			}

			// Calculate the longest axis
			const uint32_t axis = ChooseAxisForPartition(centroidsMin, centroidsMax);

			const float splitValue = centroidsMin.GetValueByAxisIndex(axis) + ((centroidsMax.GetValueByAxisIndex(axis) - centroidsMin.GetValueByAxisIndex(axis)) / 2.0f);

			auto iterator = std::partition(bvhPartitionData.begin() + start, bvhPartitionData.begin() + end + 1, [&](BVHPartitionData& data) { return data.m_centroid.m_position.GetValueByAxisIndex(axis) < splitValue; });

			const uint32_t indexOfRightNode = (uint32_t)std::distance(bvhPartitionData.begin(), iterator);

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
			Vector3 centroidsMin(bvhPartitionData[start].m_centroid.m_position);
			Vector3 centroidsMax(bvhPartitionData[start].m_centroid.m_position);

			// Calculate the bounding box for the centroids
			for (uint32_t centroidStart = start + 1u; centroidStart < end + 1u; centroidStart++)
			{
				centroidsMin = Min(centroidsMin, bvhPartitionData[centroidStart].m_centroid.m_position);
				centroidsMax = Max(centroidsMax, bvhPartitionData[centroidStart].m_centroid.m_position);
			}

			const uint32_t c_binCount = 35u;
			const uint32_t c_splitCount = c_binCount - 1u;

			float splitValues[3u * c_splitCount];
			for (uint32_t value = 0u; value < 3u * c_splitCount; value++)
			{
				splitValues[value] = INFINITY;
			}

			AABB aabbs[3u * c_splitCount * 2u];
			uint32_t triangleCounts[3u * c_splitCount * 2u] = {};

			// For every triangle
			for (uint32_t centroid = start; centroid < end + 1u; centroid++)
			{
				for (uint32_t axis = 0u; axis < 3u; axis++)
				{
					// I think you can compute the 3 axes lengths outside, and cache them, instead of needlessly recomputing here
					// this is here just for my sanity atm
					const float axisLength = centroidsMax.GetValueByAxisIndex(axis) - centroidsMin.GetValueByAxisIndex(axis);
					const float splitIncrement = axisLength / (float)c_binCount;

					for (uint32_t split = 0u; split < c_splitCount; split++)
					{
						const uint32_t splitValueIndex = (axis * c_splitCount) + split;
						splitValues[splitValueIndex] = centroidsMin.GetValueByAxisIndex(axis) + (((float)split * splitIncrement) + splitIncrement);

						const uint32_t index = (bvhPartitionData[centroid].m_centroid.m_position.GetValueByAxisIndex(axis) < splitValues[splitValueIndex]) ? 
							(axis * c_splitCount * 2u) + (split * 2u) :
							(axis * c_splitCount * 2u) + (split * 2u) + 1u;

						aabbs[index].MergeAABB(bvhPartitionData[centroid].m_aabb);
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
				for (uint32_t split = 0u; split < c_splitCount; split++)
				{
					const uint32_t lIndex = (axis * c_splitCount * 2u) + (split * 2u);
					const uint32_t rIndex = (axis * c_splitCount * 2u) + (split * 2u) + 1u;

					AABB V;
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

						const uint32_t splitValueIndex = (axis * c_splitCount) + split;
						bestSplitValue = splitValues[splitValueIndex];

						// TO DO:
						// Figure out when a splitValue will ever be set to INFINITY - Split values will never be INFINITY at this stage, they
						// would instead be set to whatever value the increment computed in the loop generating aabbs and splitvalues - this is guarranteed
					}
				}
			}

			// Partitioning
			auto iterator = std::partition(bvhPartitionData.begin() + start, bvhPartitionData.begin() + end + 1, [&](BVHPartitionData& data) { return data.m_centroid.m_position.GetValueByAxisIndex(bestAxis) < bestSplitValue; });

			const uint32_t indexOfRightNode = (uint32_t)std::distance(bvhPartitionData.begin(), iterator);

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

		const ConstructResult left = ConstructNode(bvhPartitionData, leftStart, leftEnd, bvhPartitionStrategy);
		const ConstructResult right = ConstructNode(bvhPartitionData, rightStart, rightEnd, bvhPartitionStrategy);

		m_innerNodes[innerNodeIndex].m_leftChild = left.m_index;
		m_innerNodes[innerNodeIndex].m_leftAABB = left.m_aabb;

		m_innerNodes[innerNodeIndex].m_rightChild = right.m_index;
		m_innerNodes[innerNodeIndex].m_rightAABB = right.m_aabb;

		cs.m_index = innerNodeIndex;
		cs.m_aabb.MergeAABB(left.m_aabb);
		cs.m_aabb.MergeAABB(right.m_aabb);
	}

	return cs;
}
