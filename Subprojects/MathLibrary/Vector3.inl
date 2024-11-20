#include "Vector3.h"

#include <cmath>

// --------------------------------------------------------------------------------
inline Vector3::Vector3() : m_x(0.0f), m_y(0.0f), m_z(0.0f)
{
}

// --------------------------------------------------------------------------------
inline Vector3::Vector3(float x, float y, float z) : m_x(x), m_y(y), m_z(z)
{
}

// --------------------------------------------------------------------------------
inline float Vector3::X() const
{
    return m_x;
}

// --------------------------------------------------------------------------------
inline float Vector3::Y() const
{
    return m_y;
}

// --------------------------------------------------------------------------------
inline float Vector3::Z() const
{
    return m_z;
}

// --------------------------------------------------------------------------------
inline void Vector3::SetX(float x)
{
    m_x = x;
}

// --------------------------------------------------------------------------------
inline void Vector3::SetY(float y)
{
    m_y = y;
}

// --------------------------------------------------------------------------------
inline void Vector3::SetZ(float z)
{
    m_z = z;
}

// --------------------------------------------------------------------------------
inline Vector3 operator+(const Vector3& lhs, const Vector3& rhs)
{
    return Vector3(lhs.X() + rhs.X(), lhs.Y() + rhs.Y(), lhs.Z() + rhs.Z());
}

// --------------------------------------------------------------------------------
inline Vector3 operator-(const Vector3& lhs, const Vector3& rhs)
{
    return Vector3(lhs.X() - rhs.X(), lhs.Y() - rhs.Y(), lhs.Z() - rhs.Z());
}

// --------------------------------------------------------------------------------
inline Vector3 operator*(const float scalar, const Vector3& vec)
{
    return Vector3(scalar * vec.X(), scalar * vec.Y(), scalar * vec.Z());
}

// --------------------------------------------------------------------------------
inline Vector3 operator-(const Vector3& vec)
{
    return Vector3(-vec.X(), -vec.Y(), -vec.Z());
}

// --------------------------------------------------------------------------------
inline float Dot(const Vector3& vecA, const Vector3& vecB)
{
    return (vecA.X() * vecB.X()) + (vecA.Y() * vecB.Y()) + (vecA.Z() * vecB.Z());
}

// --------------------------------------------------------------------------------
inline float Magnitude(const Vector3& vec)
{
    return sqrtf((vec.X() * vec.X()) + (vec.Y() * vec.Y()) + (vec.Z() * vec.Z()));;
}

// --------------------------------------------------------------------------------
inline Vector3 Normalize(const Vector3& vec)
{
    float c_magnitude = Magnitude(vec);
    return Vector3(vec.X() / c_magnitude, vec.Y() / c_magnitude, vec.Z() / c_magnitude);
}