#pragma once
#include "Vertex.h"
#include "Vector3.h"

struct AABB;

// --------------------------------------------------------------------------------
struct Triangle
{
	Vertex m_vertices[3u];
	Vector3 m_edge1;
	Vector3 m_edge2;
};

// --------------------------------------------------------------------------------
AABB CalculateTriangleAABB(const Triangle& triangle);

