#include "Camera.h"

// --------------------------------------------------------------------------------
Camera::Camera()
{
}

// --------------------------------------------------------------------------------
Vector3 Camera::GetCameraLocation() const
{
    return m_position;
}

// --------------------------------------------------------------------------------
void Camera::SetCameraLocation(const Vector3& translation)
{
    m_position = translation;
}
