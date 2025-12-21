#include "Model.h"

#include "ModelParser.h"

// --------------------------------------------------------------------------------
Model::Model()
{
	ModelParser* m_modelParser = new ModelParser();
	m_modelParser->ParseFile(R"(Scenes\Sponza\sponza.obj)", 1.0f, m_triangles, m_triangle4s);
	m_center = Vector3(2.88791323f, 7.37331104f, -0.183363333f); // Manual center for Sponza, as for whatever reason the center is not the center
	delete m_modelParser;
}

// --------------------------------------------------------------------------------
const Vector3& Model::GetCenter() const
{
	return m_center;
}

// --------------------------------------------------------------------------------
const std::vector<Triangle>& Model::GetTriangles() const
{
	return m_triangles;
}

// --------------------------------------------------------------------------------
const std::vector<Triangle4>& Model::GetTriangle4s() const
{
	return m_triangle4s;
}
