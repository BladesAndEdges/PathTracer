#include "BVH4AccellStructure.h"

//#include "BVHAccellStructure.h"
//#include "AABB.h"
//
//// --------------------------------------------------------------------------------
//BVH4AccellStructure::BVH4AccellStructure(const BVHAccellStructure* bvhAccellStructure)
//{
//	const ConstructResult cr = ConstructBVH4Node(bvhAccellStructure, 0u);
//}
//
//// --------------------------------------------------------------------------------
//ConstructResult BVH4AccellStructure::ConstructBVH4Node(const BVHAccellStructure* bvhAccellStructure, const uint32_t grandParentIndex)
//{	
//	// If it's a triangle, the index is negative
//	// If the triangle 
//	//Get the nodes grand children
//	const BVHInnerNode grandparent = bvhAccellStructure->GetInnerNode(grandParentIndex);
//
//	const BVHInnerNode leftParent = bvhAccellStructure->GetInnerNode(grandparent.m_leftChild);
//	const BVHInnerNode rightParent = bvhAccellStructure->GetInnerNode(grandparent.m_rightChild);
//
//	ConstructResult cr;
//
//	BVH4InnerNode bvh4Node;
//	const uint32_t index = (uint32_t)m_bvh4InnerNodes.size();
//	m_bvh4InnerNodes.push_back(bvh4Node);
//
//	const ConstructResult child0 = ConstructBVH4Node(bvhAccellStructure, leftParent.m_leftChild);
//	const ConstructResult child1 = ConstructBVH4Node(bvhAccellStructure, leftParent.m_rightChild);
//	const ConstructResult child2 = ConstructBVH4Node(bvhAccellStructure, rightParent.m_leftChild);
//	const ConstructResult child3 = ConstructBVH4Node(bvhAccellStructure, rightParent.m_leftChild);
//
//	bvh4Node.m_child[0u] = child0.m_index;
//	bvh4Node.m_child[1u] = child1.m_index;
//	bvh4Node.m_child[2u] = child2.m_index;
//	bvh4Node.m_child[3u] = child3.m_index;
//
//	bvh4Node.m_bbox[0u] = leftParent.m_leftAABB;
//	bvh4Node.m_bbox[1u] = leftParent.m_rightAABB;
//	bvh4Node.m_bbox[2u] = rightParent.m_leftAABB;
//	bvh4Node.m_bbox[3u] = rightParent.m_rightAABB;
//
//	cr.m_index = index;
//	// the aabb is irrelevant
//
//	return cr;
//}




