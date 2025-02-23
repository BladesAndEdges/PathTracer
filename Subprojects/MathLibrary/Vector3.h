#pragma once

#include <stdint.h>

class Vector3
{	
public:

	Vector3();
	Vector3(float x, float y, float z);

	inline float X() const;
	inline float Y() const;
	inline float Z() const;

	inline float GetValueByAxisIndex(uint32_t axis) const;

	inline void SetX(float x);
	inline void SetY(float y);
	inline void SetZ(float z);

	static inline Vector3 RandomVector3(float min, float max);
	static inline Vector3 RandomUnitVector3(float min, float max);
	static inline Vector3 RandomVector3OnHemisphere(const Vector3& surfaceNormal);

private:

	float m_x;
	float m_y;
	float m_z;
};

#include "Vector3.inl"