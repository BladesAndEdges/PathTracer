#include "Vector3.h"

#include "math.h"

// --------------------------------------------------------------------------------
Vector3::Vector3(float x, float y, float z) : m_x(x), m_y(y), m_z(z)
{
}

// --------------------------------------------------------------------------------
float Vector3::X() const
{
    return m_x;
}

// --------------------------------------------------------------------------------
float Vector3::Y() const
{
    return m_y;
}

// --------------------------------------------------------------------------------
float Vector3::Z() const
{
    return m_z;
}

// --------------------------------------------------------------------------------
void Vector3::SetX(float x)
{
    m_x = x;
}

// --------------------------------------------------------------------------------
void Vector3::SetY(float y)
{
    m_y = y;
}

// --------------------------------------------------------------------------------
void Vector3::SetZ(float z)
{
    m_z = z;
}

// --------------------------------------------------------------------------------
Vector3 operator+(const Vector3& lhs, const Vector3& rhs)
{
    return Vector3(lhs.X() + rhs.X(), lhs.Y() + rhs.Y(), lhs.Z() + rhs.Z());
}

// --------------------------------------------------------------------------------
Vector3 operator-(const Vector3& lhs, const Vector3& rhs)
{
    return Vector3(lhs.X() - rhs.X(), lhs.Y() - rhs.Y(), lhs.Z() - rhs.Z());
}

// --------------------------------------------------------------------------------
Vector3 operator*(const float scalar, const Vector3& vec)
{
    return Vector3(scalar * vec.X(), scalar * vec.Y(), scalar * vec.Z());
}

// --------------------------------------------------------------------------------
float Dot(const Vector3& vecA, const Vector3& vecB)
{
    return (vecA.X() * vecB.X()) + (vecA.Y() * vecB.Y()) + (vecA.Z() * vecB.Z());
}

// --------------------------------------------------------------------------------
float Magnitude(const Vector3& vec)
{
    return sqrtf((vec.X() * vec.X()) + (vec.Y() * vec.Y()) + (vec.Z() * vec.Z()));;
}
