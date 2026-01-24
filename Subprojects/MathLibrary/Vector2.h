#pragma once

class Vector2
{

public:

	Vector2();
	Vector2(float x, float y);

	inline float X() const;
	inline float Y() const;

private:

	float m_x;
	float m_y;
};

#include "Vector2.inl"