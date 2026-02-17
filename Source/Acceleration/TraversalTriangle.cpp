#include "TraversalTriangle.h"

#include <Windows.h>
#include <WinBase.h>

// --------------------------------------------------------------------------------
TraversalTriangle::TraversalTriangle()
{
	SecureZeroMemory(m_v0, 3u * sizeof(float));
	SecureZeroMemory(m_edge1, 3u * sizeof(float));
	SecureZeroMemory(m_edge2, 3u * sizeof(float));
}
