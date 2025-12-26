#include "Triangle4.h"

#include <Windows.h>
#include <WinBase.h>

// --------------------------------------------------------------------------------
Triangle4::Triangle4()
{
	SecureZeroMemory(m_v0X, 4u * sizeof(float));
	SecureZeroMemory(m_v0Y, 4u * sizeof(float));
	SecureZeroMemory(m_v0Z, 4u * sizeof(float));

	SecureZeroMemory(m_edge1X, 4u * sizeof(float));
	SecureZeroMemory(m_edge1Y, 4u * sizeof(float));
	SecureZeroMemory(m_edge1Z, 4u * sizeof(float));

	SecureZeroMemory(m_edge2X, 4u * sizeof(float));
	SecureZeroMemory(m_edge2Y, 4u * sizeof(float));
	SecureZeroMemory(m_edge2Z, 4u * sizeof(float));
}
