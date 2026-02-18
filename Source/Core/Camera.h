#pragma once

#include <Vector3.h>

class Camera
{
public:

	Camera();

	Vector3 GetCameraLocation() const;

	void SetCameraLocation(const Vector3& translation);

private:

	Vector3 m_position;
};

