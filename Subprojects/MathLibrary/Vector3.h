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

	static inline Vector3 RandomVector3(float min, float max);
	static inline Vector3 RandomUnitVector3(float min, float max);
	static inline Vector3 RandomVector3OnHemisphere(const Vector3& surfaceNormal);
	static inline Vector3 Min(const Vector3& lh, const Vector3& rh);
	static inline Vector3 Max(const Vector3& lh, const Vector3& rh);

private:

	float m_x;
	float m_y;
	float m_z;
};

#include "Vector3.inl"