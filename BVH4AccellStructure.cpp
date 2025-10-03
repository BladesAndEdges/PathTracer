#include "BVH4AccellStructure.h"

#include "BVH2AccellStructure.h"
#include "AABB.h"

// --------------------------------------------------------------------------------
BVH4AccellStructure::BVH4AccellStructure(const BVH2AccellStructure* bvh2AccellStructure)
{
	assert(bvh2AccellStructure != nullptr);
	// Add getter for the size of the internal node vector, and check it is not empty
	const uint32_t bvhRootIndex = BuildBVH4NodeFromBVH2NodeTri4(bvh2AccellStructure, 0);
	const uint32_t bv = BuildBVH4NodeFromBVH2Node(bvh2AccellStructure, 0);
	(void)bv;
	(void)bvhRootIndex;
}

// --------------------------------------------------------------------------------
void RecursiveGetChildren(const BVH2AccellStructure* bvh2AccellStructure, const uint32_t& root, const AABB& box, const uint32_t depth, uint32_t* children, 
	AABB* boxes, uint32_t& addedChildren, uint32_t& triangleMask)
{
	assert(bvh2AccellStructure != nullptr);
	assert(depth <= 2u);
	assert(children != nullptr);
	assert(addedChildren < 4u);
	assert(triangleMask <= 15u);

	const bool isTriangle = root >> 31u;

	if (isTriangle || (depth > 1u))
	{
		children[addedChildren] = root;
		boxes[addedChildren] = box;

		if (isTriangle)
		{
			triangleMask = triangleMask | (1u << (3u - addedChildren));
			assert(triangleMask <= 15u);
		}

		addedChildren++;
	}
	else
	{
		const BVH2InnerNode rootNode = bvh2AccellStructure->GetInnerNode(root);

		RecursiveGetChildren(bvh2AccellStructure, rootNode.m_leftChild, rootNode.m_leftAABB, depth + 1, children, boxes, addedChildren, triangleMask);
		RecursiveGetChildren(bvh2AccellStructure, rootNode.m_rightChild, rootNode.m_rightAABB, depth + 1, children, boxes, addedChildren, triangleMask);
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
	uint32_t triangleMask = 0u;

	RecursiveGetChildren(bvh2AccellStructure, bvh2SubtreeRootIndex, AABB(), 0, children, boxes, addedChildren, triangleMask);
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
uint32_t BVH4AccellStructure::BuildBVH4NodeFromBVH2NodeTri4(const BVH2AccellStructure* bvh2AccellStructure, const uint32_t bvh2SubtreeRootIndex)
{
	assert((bvh2SubtreeRootIndex >> 31u) != 1u);

	uint32_t children[4u];
	AABB boxes[4u];
	uint32_t addedChildren = 0u;
	uint32_t triangleMask = 0u;
	RecursiveGetChildren(bvh2AccellStructure, bvh2SubtreeRootIndex, AABB(), 0u, children, boxes, addedChildren, triangleMask);

	const uint32_t bvh4InnerNodeIndex = (uint32_t)m_innerNodesTri4.size();
	m_innerNodesTri4.push_back(BVH4InnerNode());

	for (uint32_t child = 0u; child < addedChildren; child++)
	{
		const uint32_t postShiftValue = triangleMask >> (3u - child);
		if (postShiftValue & 1u)
		{
			Triangle4 triangle4;
			
			const uint32_t triangleIndex = children[child] & ~(1u << 31u);
			const Triangle triangle = bvh2AccellStructure->GetTriangle(triangleIndex);

			triangle4.m_edge1X[0u] = triangle.m_vertices[1u].m_position[0u] - triangle.m_vertices[0u].m_position[0u];
			triangle4.m_edge1Y[0u] = triangle.m_vertices[1u].m_position[1u] - triangle.m_vertices[0u].m_position[1u];
			triangle4.m_edge1Z[0u] = triangle.m_vertices[1u].m_position[2u] - triangle.m_vertices[0u].m_position[2u];

			triangle4.m_edge2X[0u] = triangle.m_vertices[2u].m_position[0u] - triangle.m_vertices[0u].m_position[0u];
			triangle4.m_edge2Y[0u] = triangle.m_vertices[2u].m_position[1u] - triangle.m_vertices[0u].m_position[1u];
			triangle4.m_edge2Z[0u] = triangle.m_vertices[2u].m_position[2u] - triangle.m_vertices[0u].m_position[2u];

			triangle4.m_v0X[0u] = triangle.m_vertices[0u].m_position[0u];
			triangle4.m_v0Y[0u] = triangle.m_vertices[0u].m_position[1u];
			triangle4.m_v0Z[0u] = triangle.m_vertices[0u].m_position[2u];

			const uint32_t triangle4Index = (uint32_t)m_triangle4s.size();
			m_triangle4s.push_back(triangle4);

			m_innerNodesTri4[bvh4InnerNodeIndex].m_child[child] = triangle4Index | (1u << 31u);

			m_innerNodesTri4[bvh4InnerNodeIndex].m_aabbMinX[child] = boxes[child].m_min.X();
			m_innerNodesTri4[bvh4InnerNodeIndex].m_aabbMinY[child] = boxes[child].m_min.Y();
			m_innerNodesTri4[bvh4InnerNodeIndex].m_aabbMinZ[child] = boxes[child].m_min.Z();
			m_innerNodesTri4[bvh4InnerNodeIndex].m_aabbMaxX[child] = boxes[child].m_max.X();
			m_innerNodesTri4[bvh4InnerNodeIndex].m_aabbMaxY[child] = boxes[child].m_max.Y();
			m_innerNodesTri4[bvh4InnerNodeIndex].m_aabbMaxZ[child] = boxes[child].m_max.Z();

		}
		else
		{
			m_innerNodesTri4[bvh4InnerNodeIndex].m_child[child] = BuildBVH4NodeFromBVH2NodeTri4(bvh2AccellStructure, children[child]);
			
			m_innerNodesTri4[bvh4InnerNodeIndex].m_aabbMinX[child] = boxes[child].m_min.X();
			m_innerNodesTri4[bvh4InnerNodeIndex].m_aabbMinY[child] = boxes[child].m_min.Y();
			m_innerNodesTri4[bvh4InnerNodeIndex].m_aabbMinZ[child] = boxes[child].m_min.Z();

			m_innerNodesTri4[bvh4InnerNodeIndex].m_aabbMaxX[child] = boxes[child].m_max.X();
			m_innerNodesTri4[bvh4InnerNodeIndex].m_aabbMaxY[child] = boxes[child].m_max.Y();
			m_innerNodesTri4[bvh4InnerNodeIndex].m_aabbMaxZ[child] = boxes[child].m_max.Z();
		}
	}

	//Populate the remaining space with dummy nodes
	for (uint32_t child = addedChildren; child < 4u; child++)
	{
		m_innerNodesTri4[bvh4InnerNodeIndex].m_child[child] = 0x7fffffffu;

		m_innerNodesTri4[bvh4InnerNodeIndex].m_aabbMinX[child] = std::nanf("");
		m_innerNodesTri4[bvh4InnerNodeIndex].m_aabbMinY[child] = std::nanf("");
		m_innerNodesTri4[bvh4InnerNodeIndex].m_aabbMinZ[child] = std::nanf("");

		m_innerNodesTri4[bvh4InnerNodeIndex].m_aabbMaxX[child] = std::nanf("");
		m_innerNodesTri4[bvh4InnerNodeIndex].m_aabbMaxY[child] = std::nanf("");
		m_innerNodesTri4[bvh4InnerNodeIndex].m_aabbMaxZ[child] = std::nanf("");
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
const BVH4InnerNode BVH4AccellStructure::GetInnerNodeTri4(const uint32_t index) const
{
	assert(index < m_innerNodesTri4.size());
	return m_innerNodesTri4[index];
}

// --------------------------------------------------------------------------------
const Triangle4 BVH4AccellStructure::GetTriangle4(const uint32_t index) const
{
	assert(index < (uint32_t)m_triangle4s.size());
	return m_triangle4s[index];
}

// --------------------------------------------------------------------------------
const uint32_t BVH4AccellStructure::GetNumInnderNodes() const
{
	return (uint32_t)m_innerNodes.size();
}