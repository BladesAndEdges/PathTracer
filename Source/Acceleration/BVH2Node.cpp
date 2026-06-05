#include "BVH2Node.h"
#include <cmath>

// --------------------------------------------------------------------------------
BVH2Node::BVH2Node() 
    : m_leftChild(0x7fffffffu), 
    m_rightChild(0x7fffffffu),
    m_leftAABB(Vector3(std::nanf(""), std::nanf(""), std::nanf("")), Vector3(std::nanf(""), std::nanf(""), std::nanf(""))),
    m_rightAABB(Vector3(std::nanf(""), std::nanf(""), std::nanf("")), Vector3(std::nanf(""), std::nanf(""), std::nanf(""))),
    padding0(UINT32_MAX), 
    padding1(UINT32_MAX)
{
}
