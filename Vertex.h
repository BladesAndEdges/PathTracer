#pragma once

#include<string>
#include <unordered_set>

// --------------------------------------------------------------------------------
struct Vertex
{
	float m_position[3];

	Vertex();
	Vertex(float x, float y, float z);
};

bool operator==(const Vertex& lhs, const Vertex& rhs);

// --------------------------------------------------------------------------------
// class for hash function 
struct KeyHasher
{
	std::size_t operator()(const Vertex& v) const
	{
		using std::size_t;
		using std::hash;
		using std::string;

		// Currently only uses position, change this to the original, later on
		return ((hash<float>()(v.m_position[0])
			^ (hash<float>()(v.m_position[1]) << 1)) >> 1)
			^ (hash<float>()(v.m_position[2]) << 1);
	}
};

