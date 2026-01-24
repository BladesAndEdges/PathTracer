#pragma once

#include<string>
#include <unordered_set>

// --------------------------------------------------------------------------------
struct Vertex
{
	float m_position[3u];
	float m_textureCoordinate[2u];

	Vertex();
	Vertex(float x, float y, float z);
};

bool operator==(const Vertex& lhs, const Vertex& rhs);

