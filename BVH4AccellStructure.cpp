#include "BVH4AccellStructure.h"

#include "BVH2AccellStructure.h"
#include "AABB.h"

// --------------------------------------------------------------------------------
BVH4AccellStructure::BVH4AccellStructure(const BVH2AccellStructure* bvh2AccellStructure)
{
	assert(bvh2AccellStructure != nullptr);
	// Add getter for the size of the internal node vector, and check it is not empty
	const uint32_t bvhRootIndex = BuildBVH4NodeFromBVH2Node(bvh2AccellStructure, 0);
	assert(bvh == 0u);
	(void)bvhRootIndex;
}

void RecursiveGetChildren(const BVH2AccellStructure* bvh2AccellStructure, const uint32_t& root, const AABB& box, const uint32_t depth, uint32_t* children, AABB* boxes,  uint32_t& addedChildren)
{
	assert(bvh2AccellStructure != nullptr);
	assert(depth < 2u);
	assert(children != nullptr);
	assert(addedChildren < 4u);

	const bool isTriangle = root >> 31u;

	if (isTriangle || depth > 1u)
	{
		*children = root;
		*boxes = box;
		addedChildren++;
	}
	else
	{
		const BVH2InnerNode rootNode = bvh2AccellStructure->GetInnerNode(root);

		RecursiveGetChildren(bvh2AccellStructure, rootNode.m_leftChild, rootNode.m_leftAABB, depth + 1, children + addedChildren, boxes + addedChildren, addedChildren);
		RecursiveGetChildren(bvh2AccellStructure, rootNode.m_rightChild, rootNode.m_rightAABB, depth + 1, children + addedChildren, boxes + addedChildren, addedChildren);
	}
}

// --------------------------------------------------------------------------------
uint32_t BVH4AccellStructure::BuildBVH4NodeFromBVH2Node(const BVH2AccellStructure* bvh2AccellStructure, const uint32_t subtreeRootIndex)
{
	// Get children in local subtree
	uint32_t children[4u];
	AABB boxes[4u];
	uint32_t addedChildren = 0u;
	RecursiveGetChildren(bvh2AccellStructure, subtreeRootIndex, AABB(), 0, children, boxes, addedChildren);
	assert(addedChildren >= 1u);
	assert(addedChildren <= 4u);

	BVH4InnerNode node;
	const uint32_t bvh4InnerNodeIndex = (uint32_t)m_bvh4InnerNodes.size();
	m_bvh4InnerNodes.push_back(node);

	for (uint32_t childIndex = 0; childIndex < addedChildren; childIndex++)
	{
		node.m_child[childIndex] = BuildBVH4NodeFromBVH2Node(bvh2AccellStructure, children[childIndex]);
		node.m_bbox[childIndex] = boxes[childIndex];
	}

	// Create dummies
	for (uint32_t childIndex = addedChildren; childIndex < 4u; childIndex++)
	{
		node.m_child[childIndex] = 0x7ffffff;
		node.m_bbox[childIndex] = AABB();
	}

	return bvh4InnerNodeIndex;
}
