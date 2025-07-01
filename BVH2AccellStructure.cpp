#include "BVH2AccellStructure.h"

#include <Windows.h>
#include <assert.h>
#include <algorithm>
#include <cmath>

#define EPSILON 0.00001f

// --------------------------------------------------------------------------------
BVH2AccellStructure::BVH2AccellStructure(const std::vector<Triangle>& triangles, const BVH2PartitionStrategy& bvhPartitionStrategy) : m_triangles(triangles)
{
	assert(triangles.size() != 0);

	// To make it easier to test for now
	const uint32_t dataCount = (uint32_t)triangles.size();

	std::vector<BVHTriangleData> bvhTriangleData;
	bvhTriangleData.resize(dataCount);
	
	for (uint32_t triangle = 0u; triangle < dataCount; triangle++)
	{
		bvhTriangleData[triangle].m_aabb = CalculateTriangleAABB(triangles[triangle]);
		
		bvhTriangleData[triangle].m_centroid.m_triangleIndex = triangle;
		bvhTriangleData[triangle].m_centroid.m_position = 0.5f * (bvhTriangleData[triangle].m_aabb.m_min + bvhTriangleData[triangle].m_aabb.m_max);
	}
	
	const ConstructResult cr = ConstructNode(bvhTriangleData.data(), dataCount, bvhPartitionStrategy);
}

// --------------------------------------------------------------------------------
const BVH2InnerNode BVH2AccellStructure::GetInnerNode(uint32_t index) const
{
	assert(index < (uint32_t)m_innerNodes.size());
	return m_innerNodes[index];
}

// --------------------------------------------------------------------------------
uint32_t BVH2AccellStructure::GetNodeCount() const
{
	return (uint32_t)m_innerNodes.size();
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
ConstructResult BVH2AccellStructure::ConstructNode(BVHTriangleData* bvhData, const uint32_t count, const BVH2PartitionStrategy& bvhPartitionStrategy)
{
	ConstructResult cr;

	if (count == 1u)
	{
		if (m_innerNodes.size() == 0u)
		{
			BVH2InnerNode node;
			const uint32_t innerNodeIndex = (uint32_t)m_innerNodes.size();
			m_innerNodes.push_back(node);

			m_innerNodes[innerNodeIndex].m_leftChild = bvhData->m_centroid.m_triangleIndex | (1u << 31u);
			m_innerNodes[innerNodeIndex].m_leftAABB = bvhData->m_aabb;
		}

		cr.m_index = bvhData->m_centroid.m_triangleIndex | (1u << 31u);
		cr.m_aabb = bvhData->m_aabb;
	}
	else
	{
		BVH2InnerNode node;
		const uint32_t innerNodeIndex = (uint32_t)m_innerNodes.size();
		m_innerNodes.push_back(node);

		BVHTriangleData* leftStart = nullptr;;
		BVHTriangleData* rightStart = nullptr;
		uint32_t leftCount = UINT32_MAX;
		uint32_t rightCount = UINT32_MAX;

		switch (bvhPartitionStrategy)
		{
		case BVH2PartitionStrategy::HalfWayPoint:
		{
			leftStart = bvhData;
			leftCount = count / 2u;

			rightStart = bvhData + leftCount;
			rightCount = count - leftCount;

			break;
		}
		case BVH2PartitionStrategy::HalfWayLongestAxis:
		{
			if (count == 2u)
			{
				leftStart = bvhData;
				leftCount = 1u;
				rightStart = bvhData + 1u;
				rightCount = 1u;

				break;
			}

			// TODO: Maybe make a Min/Max for Vector3 to make this easier
			Vector3 centroidsMin(bvhData->m_centroid.m_position);
			Vector3 centroidsMax(bvhData->m_centroid.m_position);
		
			// Calculate the bounding box for the centroids
			for (uint32_t centroid = 1u; centroid < count; centroid++)
			{
				centroidsMin = Min(centroidsMin, (bvhData + centroid)->m_centroid.m_position);
				centroidsMax = Max(centroidsMax, (bvhData + centroid)->m_centroid.m_position);
			}
		
			// Calculate the longest axis
			const uint32_t axis = ChooseAxisForPartition(centroidsMin, centroidsMax);
		
			const float splitValue = centroidsMin.GetValueByAxisIndex(axis) + ((centroidsMax.GetValueByAxisIndex(axis) - centroidsMin.GetValueByAxisIndex(axis)) / 2.0f);
		
			//There might be a + 1 on the end condition, keep in mind that
			auto iterator = std::partition(bvhData, bvhData + count, [&](BVHTriangleData& data) { return data.m_centroid.m_position.GetValueByAxisIndex(axis) < splitValue; });
		
			const uint32_t firstTriangleInRightNode = (uint32_t)std::distance(bvhData, iterator);

			leftStart = bvhData;
			leftCount = firstTriangleInRightNode;
			rightStart = bvhData + firstTriangleInRightNode;
			rightCount = count - firstTriangleInRightNode;

			// If all nodes end up on either left or right subtree
			// Use halfway split instead
			if ((leftCount == 0u) || (rightCount == 0u))
			{
				leftStart = bvhData;
				leftCount = count / 2u;

				rightStart = bvhData + leftCount;
				rightCount = count - leftCount;
			}
		
			break;
		}
		case BVH2PartitionStrategy::HalfWayLongestAxisWithSAH:
		{
			if (count == 2u)
			{
				leftStart = bvhData;
				leftCount = 1u;
				rightStart = bvhData + 1u;
				rightCount = 1u;

				break;
			}

			// Compute the bounds around the centroids at this node
			Vector3 centroidsMin(bvhData->m_centroid.m_position);
			Vector3 centroidsMax(bvhData->m_centroid.m_position);
		
			for (uint32_t centroid = 1u; centroid < count; centroid++)
			{
				centroidsMin = Min(centroidsMin, (bvhData + centroid)->m_centroid.m_position);
				centroidsMax = Max(centroidsMax, (bvhData + centroid)->m_centroid.m_position);
			}
		
			// SAH values
			float bestCost = INFINITY;
			float bestCostK1 = INFINITY;
			uint32_t bestAxis = UINT32_MAX;
			uint32_t rightBinStart = UINT32_MAX; // Range of right side is from [rightBinStart, end - 1]
		
			// For debugging purposes
			uint32_t expectedLeftTriangleCount = 0u;
			uint32_t expectedRightTriangleCount = 0u;
		
			constexpr uint32_t c_binCount = 35u;
			constexpr uint32_t c_axisCount = 3u;
		
			for (uint32_t axis = 0u; axis < c_axisCount; axis++)
			{
				uint32_t binTriangleCount[c_binCount];
				ZeroMemory(binTriangleCount, sizeof(binTriangleCount));
		
				AABB binAABB[c_binCount];
		
				const float centroidAxisMin = centroidsMin.GetValueByAxisIndex(axis);
				const float centroidAxisMax = centroidsMax.GetValueByAxisIndex(axis);
		
				const float k1 = ((float)c_binCount * (1.0f - EPSILON)) / (centroidAxisMax - centroidAxisMin);
		
				// Centroid-to-bin-projection as in section 3.1 of https://www.sci.utah.edu/~wald/Publications/2007/FastBuild/download/fastbuild.pdf
				for (uint32_t centroid = 0u; centroid < count; centroid++)
				{
					const uint32_t binId = (uint32_t)(k1 * ((bvhData + centroid)->m_centroid.m_position.GetValueByAxisIndex(axis) - centroidAxisMin));
		
					assert(binId < c_binCount);
		
					// We keep track of both the number of triangle for that bin along that axis, and the bin bounds
					binTriangleCount[binId]++;
					binAABB[binId].MergeAABB((bvhData + centroid)->m_aabb);
				}
		
				// Calculates the left and right aabb and triangle count
				for (uint32_t split = 0u; split < c_binCount - 1u; split++)
				{
					const uint32_t leftBinCount = 1u + split;
					const uint32_t rightBinCount = c_binCount - leftBinCount;
		
					assert(rightBinCount > 0u);
		
					// Union of left bins
					uint32_t leftTriangleCount = 0u;
					AABB leftAABB;
					for (uint32_t leftBin = 0u; leftBin < leftBinCount; leftBin++)
					{
						leftTriangleCount += binTriangleCount[leftBin];
						leftAABB.MergeAABB(binAABB[leftBin]);
					}
		
					// Union of right bins
					uint32_t rightTriangleCount = 0u;
					AABB rightAABB;
					for (uint32_t rightBin = 0u; rightBin < rightBinCount; rightBin++)
					{
						rightTriangleCount += binTriangleCount[leftBinCount + rightBin];
						rightAABB.MergeAABB(binAABB[leftBinCount + rightBin]);
					}
		
					AABB V;
					V.MergeAABB(leftAABB);
					V.MergeAABB(rightAABB);
		
					const float saV = V.GetSurfaceArea();
		
					float vl = 0.0f;
					float vlSah = 0.0f;
					if (leftTriangleCount > 0u)
					{
						// Division by infinity
						// leftAABB.m_max is infinity here
						vl = leftAABB.GetSurfaceArea();
						vlSah = vl / saV;
					}
		
					float vr = 0.0f;
					float vrSah = 0.0f;
					if (rightTriangleCount > 0u)
					{
						vr = rightAABB.GetSurfaceArea();
						vrSah = vr / saV;
					}
		
					const float cost = vlSah * (float)leftTriangleCount +
						vrSah * (float)rightTriangleCount;
		
					if (cost < bestCost)
					{
						bestCost = cost;
						bestCostK1 = k1;
						bestAxis = axis;
		
						rightBinStart = leftBinCount;
		
						expectedLeftTriangleCount = leftTriangleCount;
						expectedRightTriangleCount = rightTriangleCount;
					}
				}
			}

			auto iterator = std::partition(bvhData, bvhData + count, [&](BVHTriangleData& data) { return (uint32_t)(bestCostK1 * (data.m_centroid.m_position.GetValueByAxisIndex(bestAxis) - centroidsMin.GetValueByAxisIndex(bestAxis))) < rightBinStart; });

			const uint32_t firstTriangleInRightNode = (uint32_t)std::distance(bvhData, iterator);

			leftStart = bvhData;
			leftCount = firstTriangleInRightNode;
			rightStart = bvhData + firstTriangleInRightNode;
			rightCount = count - firstTriangleInRightNode;

			// If all nodes end up on either left or right subtree
			// Use halfway split instead
			if ((leftCount == 0u) || (rightCount == 0u))
			{
				leftStart = bvhData;
				leftCount = count / 2u;

				rightStart = bvhData + leftCount;
				rightCount = count - leftCount;
			}

			break;
		}
		default:
		{
			leftStart = bvhData;
			leftCount = count / 2u;
			rightStart = bvhData + leftCount;
			rightCount = count - leftCount;

			break;
		}
		}

		const ConstructResult left = ConstructNode(leftStart, leftCount, bvhPartitionStrategy);
		const ConstructResult right = ConstructNode(rightStart, rightCount, bvhPartitionStrategy);

		m_innerNodes[innerNodeIndex].m_leftChild = left.m_index;
		m_innerNodes[innerNodeIndex].m_leftAABB = left.m_aabb;

		m_innerNodes[innerNodeIndex].m_rightChild = right.m_index;
		m_innerNodes[innerNodeIndex].m_rightAABB = right.m_aabb;

		cr.m_index = innerNodeIndex;
		cr.m_aabb.MergeAABB(left.m_aabb);
		cr.m_aabb.MergeAABB(right.m_aabb);
	}

	return cr;
}
