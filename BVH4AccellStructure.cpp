#include "BVH4AccellStructure.h"

#include "BVH2AccellStructure.h"
#include "AABB.h"

// --------------------------------------------------------------------------------
BVH4AccellStructure::BVH4AccellStructure(const BVH2AccellStructure* bvh2AccellStructure)
{
	assert(bvh2AccellStructure != nullptr);
	// Add getter for the size of the internal node vector, and check it is not empty
	const uint32_t bvhRootIndex = BuildBVH4NodeFromBVH2Node(bvh2AccellStructure, 0);
	assert(bvhRootIndex == 0u);
	(void)bvhRootIndex;
}

void RecursiveGetChildren(const BVH2AccellStructure* bvh2AccellStructure, const uint32_t& root, const AABB& box, const uint32_t depth, uint32_t* children, AABB* boxes, uint32_t& addedChildren)
{
	assert(bvh2AccellStructure != nullptr);
	assert(depth <= 2u);
	assert(children != nullptr);
	assert(addedChildren < 4u);

	const bool isTriangle = root >> 31u;

	if (isTriangle || (depth > 1u))
	{
		children[addedChildren] = root;
		boxes[addedChildren] = box;
		addedChildren++;
	}
	else
	{
		const BVH2InnerNode rootNode = bvh2AccellStructure->GetInnerNode(root);

		RecursiveGetChildren(bvh2AccellStructure, rootNode.m_leftChild, rootNode.m_leftAABB, depth + 1, children, boxes, addedChildren);
		RecursiveGetChildren(bvh2AccellStructure, rootNode.m_rightChild, rootNode.m_rightAABB, depth + 1, children, boxes, addedChildren);
	}
}

// --------------------------------------------------------------------------------
uint32_t BVH4AccellStructure::BuildBVH4NodeFromBVH2Node(const BVH2AccellStructure* bvh2AccellStructure, const uint32_t bvh2SubtreeRootIndex)
{
	if (bvh2SubtreeRootIndex >> 31u)
	{
		return bvh2SubtreeRootIndex;
	}

	// Get children in local subtree
	uint32_t children[4u];
	AABB boxes[4u];
	uint32_t addedChildren = 0u;

	RecursiveGetChildren(bvh2AccellStructure, bvh2SubtreeRootIndex, AABB(), 0, children, boxes, addedChildren);
	assert(addedChildren >= 1u);
	assert(addedChildren <= 4u);

	BVH4InnerNode node;
	const uint32_t bvh4InnerNodeIndex = (uint32_t)m_innerNodes.size();
	m_innerNodes.push_back(node);

	for (uint32_t childIndex = 0; childIndex < addedChildren; childIndex++)
	{
		m_innerNodes[bvh4InnerNodeIndex].m_child[childIndex] = BuildBVH4NodeFromBVH2Node(bvh2AccellStructure, children[childIndex]);

		m_innerNodes[bvh4InnerNodeIndex].m_aabbMinX[childIndex] = boxes[childIndex].m_min.X();
		m_innerNodes[bvh4InnerNodeIndex].m_aabbMinY[childIndex] = boxes[childIndex].m_min.Y();
		m_innerNodes[bvh4InnerNodeIndex].m_aabbMinZ[childIndex] = boxes[childIndex].m_min.Z();

		m_innerNodes[bvh4InnerNodeIndex].m_aabbMaxX[childIndex] = boxes[childIndex].m_max.X();
		m_innerNodes[bvh4InnerNodeIndex].m_aabbMaxY[childIndex] = boxes[childIndex].m_max.Y();
		m_innerNodes[bvh4InnerNodeIndex].m_aabbMaxZ[childIndex] = boxes[childIndex].m_max.Z();
	}

	// Create dummies
	for (uint32_t childIndex = addedChildren; childIndex < 4u; childIndex++)
	{
		m_innerNodes[bvh4InnerNodeIndex].m_child[childIndex] = 0x7fffffffu;

		m_innerNodes[bvh4InnerNodeIndex].m_aabbMinX[childIndex] = std::nanf("");
		m_innerNodes[bvh4InnerNodeIndex].m_aabbMinY[childIndex] = std::nanf("");
		m_innerNodes[bvh4InnerNodeIndex].m_aabbMinZ[childIndex] = std::nanf("");
																
		m_innerNodes[bvh4InnerNodeIndex].m_aabbMaxX[childIndex] = std::nanf("");
		m_innerNodes[bvh4InnerNodeIndex].m_aabbMaxY[childIndex] = std::nanf("");
		m_innerNodes[bvh4InnerNodeIndex].m_aabbMaxZ[childIndex] = std::nanf("");
	}

	return bvh4InnerNodeIndex;
}

// --------------------------------------------------------------------------------
const BVH4InnerNode BVH4AccellStructure::GetInnerNode(const uint32_t index) const
{
	assert(index < (uint32_t)m_innerNodes.size());
	return m_innerNodes[index];
}

// --------------------------------------------------------------------------------
const uint32_t BVH4AccellStructure::GetNumInnderNodes() const
{
	return (uint32_t)m_innerNodes.size();
}
