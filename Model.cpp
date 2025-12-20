#include "Model.h"

#include "ModelParser.h"

// --------------------------------------------------------------------------------
Model::Model()
{
	m_modelParser = new ModelParser();
	m_modelParser->ParseFile(R"(Scenes\Sponza\sponza.obj)", 1.0f, m_triangles);
}

// --------------------------------------------------------------------------------
const std::vector<Triangle>& Model::GetTriangles() const
{
	return m_triangles;
}

// --------------------------------------------------------------------------------
const std::vector<Triangle4>& Model::GetTriangle4s() const
{
	return m_modelParser->GetTriangle4Data();
}

// --------------------------------------------------------------------------------
const Vector3& Model::GetCenter() const
{
	return m_center;
}

// --------------------------------------------------------------------------------
Model::~Model()
{
	delete m_modelParser;
}
