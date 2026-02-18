#pragma once
#include "Vertex.h"
#include "Vector3.h"

struct AABB;

// --------------------------------------------------------------------------------
struct Triangle
{
	Vertex m_vertices[3u];
};

// --------------------------------------------------------------------------------
AABB CalculateTriangleAABB(const Triangle& triangle);

