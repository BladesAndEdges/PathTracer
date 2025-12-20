#pragma once

#include "Triangle.h"
#include "Triangle4.h"
#include "Vector3.h"

class ModelParser;

// --------------------------------------------------------------------------------
class Model
{
public:

	Model();

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

	const Vector3& GetCenter() const;
	const std::vector<Triangle>& GetTriangles() const;
	const std::vector<Triangle4>& GetTriangle4s() const;

	~Model();

private:

	ModelParser* m_modelParser;

	Vector3 m_center;
	std::vector<Triangle> m_triangles;
	std::vector<Triangle4> m_triangle4s;
};

