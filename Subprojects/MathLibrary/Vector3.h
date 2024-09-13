#pragma once

class Vector3
{	
public:

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

Vector3 operator+(const Vector3& lhs, const Vector3& rhs);
Vector3 operator-(const Vector3& lhs, const Vector3& rhs);
Vector3 operator*(const float scalar, const Vector3& vec);

float Dot(const Vector3& vecA, const Vector3& vecB);
float Magnitude(const Vector3& vecA, const Vector3& vecB);