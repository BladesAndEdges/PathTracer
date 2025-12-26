#pragma once

#include "Triangle.h"
#include "Vector3.h"

// --------------------------------------------------------------------------------
class Model
{
public:

	Model();

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

	const Vector3& GetCenter() const;
	const std::vector<Triangle>& GetTriangles() const;

private:

	Vector3 m_center;
	std::vector<Triangle> m_triangles;
};

