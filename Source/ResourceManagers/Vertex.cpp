#include "Vertex.h"

// --------------------------------------------------------------------------------
Vertex::Vertex() : m_position{ 0.0f, 0.0f, 0.0f }
{
}

// --------------------------------------------------------------------------------
Vertex::Vertex(float x, float y, float z) : m_position{ x, y, z }
{
}

// --------------------------------------------------------------------------------
bool operator==(const Vertex& lhs, const Vertex& rhs)
{
	bool positionEqual = false;

	for (unsigned int positionElement = 0; positionElement < 3; positionElement++)
	{
		// Position equality
		if (lhs.m_position[positionElement] == rhs.m_position[positionElement])
		{
			positionEqual = true;
		}
		else
		{
			positionEqual = false;
			break;
		}
	}

	return positionEqual;
}