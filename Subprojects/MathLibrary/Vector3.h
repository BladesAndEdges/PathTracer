#pragma once

class Vector3
{	
public:

	Vector3();
	Vector3(float x, float y, float z);

	float X() const;
	float Y() const;
	float Z() const;

	void SetX(float x);
	void SetY(float y);
	void SetZ(float z);

private:

	float m_x;
	float m_y;
	float m_z;
};

#include "Vector3.inl"