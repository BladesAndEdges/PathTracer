#include "SceneManager.h"

#include <assert.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "MaterialManager.h"
#include "Triangle.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vertex.h"


// --------------------------------------------------------------------------------
SceneManager::SceneManager(const std::string& objFile, const std::string& mtlFile) : m_materialManager(nullptr)
{
	std::filesystem::path oFile = objFile;
	std::filesystem::path mFile = mtlFile;

	const bool validObjFile = (oFile.extension() == ".obj" || oFile.extension() == ".OBJ");
	const bool validMtlFile = (mFile.extension() == ".mtl" || mFile.extension() == ".MTL");

	if (validObjFile && validMtlFile)
	{
		m_materialManager = new MaterialManager(mtlFile.data());
		Load(objFile);
		// assert if the two arrays match
	}
	else
	{
		assert(false);
	}
}

// --------------------------------------------------------------------------------
const std::vector<Triangle>& SceneManager::GetTriangles() const
{
	return m_triangles;
}

// --------------------------------------------------------------------------------
void SceneManager::Load(const std::string& objFile)
{
	const std::string path = "./Scenes/Sponza/" + objFile;

	std::ifstream ifs(path);

	if (ifs.fail())
	{
		assert(false);
	}

	std::vector<Vector3> positions;
	std::vector<Vector2> textureCoordinates;

	uint32_t materialId = UINT32_MAX;
	std::string word;
	while (ifs >> word)
	{
		if (word == "usemtl")
		{
			std::string materialName;
			ifs >> materialName;

			materialId = m_materialManager->GetMaterialIndexByName(materialName);
		}

		if (word == "v")
		{
			float x, y, z;
			ifs >> x >> y >> z;
			positions.push_back(Vector3(x,y,z));
		}

		if (word == "vt")
		{
			float x, y;
			ifs >> x >> y;
			textureCoordinates.push_back(Vector2(x, y));
		}

		if (word == "f")
		{
			std::string line;
			std::getline(ifs, line);

			std::vector<Vertex> triangleVertices;
			CreateVertices(line, positions, textureCoordinates, triangleVertices);
			std::vector<Triangle> triangles;
			Triangulate(triangleVertices, triangles);

			for (uint32_t i = 0u; i < triangles.size(); i++)
			{
				triangles[i].m_edge1.SetX(triangles[i].m_vertices[1u].m_position[0u] - triangles[i].m_vertices[0u].m_position[0u]);
				triangles[i].m_edge1.SetY(triangles[i].m_vertices[1u].m_position[1u] - triangles[i].m_vertices[0u].m_position[1u]);
				triangles[i].m_edge1.SetZ(triangles[i].m_vertices[1u].m_position[2u] - triangles[i].m_vertices[0u].m_position[2u]);

				triangles[i].m_edge2.SetX(triangles[i].m_vertices[2u].m_position[0u] - triangles[i].m_vertices[0u].m_position[0u]);
				triangles[i].m_edge2.SetY(triangles[i].m_vertices[2u].m_position[1u] - triangles[i].m_vertices[0u].m_position[1u]);
				triangles[i].m_edge2.SetZ(triangles[i].m_vertices[2u].m_position[2u] - triangles[i].m_vertices[0u].m_position[2u]);
			}

			m_triangles.insert(std::end(m_triangles), std::begin(triangles), std::end(triangles));
		}
	}
}

// --------------------------------------------------------------------------------
void SceneManager::CreateVertices(const std::string& line, const std::vector<Vector3>& positions,
	const std::vector<Vector2>& textureCoordinate, std::vector<Vertex>& out_vertices) const
{
	std::istringstream iss(line);
	std::string vertexData;

	Vertex vertex;

	int vertexPosIndex = INT_MAX;
	int vertexTexCoordIndex = INT_MAX;

	while (iss.good())
	{
		std::getline(iss, vertexData, ' ');

		if (vertexData != "")
		{
			const size_t firstBackSlashIndex = vertexData.find("/");
			const size_t secondBackSlashIndex = vertexData.find("/", firstBackSlashIndex + 1);

			vertexPosIndex = std::stoi(vertexData.substr(0, firstBackSlashIndex), nullptr);
			
			if (firstBackSlashIndex != std::string::npos)
			{
				vertexTexCoordIndex = std::stoi(vertexData.substr(firstBackSlashIndex + 1, 
					secondBackSlashIndex - firstBackSlashIndex - 1), nullptr);
			}

			if (vertexPosIndex != INT_MAX)
			{
				if (vertexPosIndex >= 1)
				{
					vertex.m_position[0] = positions[vertexPosIndex - 1].X();
					vertex.m_position[1] = positions[vertexPosIndex - 1].Y();
					vertex.m_position[2] = positions[vertexPosIndex - 1].Z();
				}
				else
				{
					const uint32_t arraySize = (uint32_t)positions.size();

					vertex.m_position[0] = positions[arraySize + vertexPosIndex].X();
					vertex.m_position[1] = positions[arraySize + vertexPosIndex].Y();
					vertex.m_position[2] = positions[arraySize + vertexPosIndex].Z();
				}
			}

			if (vertexTexCoordIndex != INT_MAX)
			{
				if (vertexTexCoordIndex >= 1)
				{
					vertex.m_textureCoordinate[0] = textureCoordinate[vertexTexCoordIndex - 1].X();
					vertex.m_textureCoordinate[1] = textureCoordinate[vertexTexCoordIndex - 1].Y();
				}
				else
				{
					const uint32_t arraySize = (unsigned int)textureCoordinate.size();

					vertex.m_textureCoordinate[0] = textureCoordinate[arraySize + vertexTexCoordIndex].X();
					vertex.m_textureCoordinate[1] = textureCoordinate[arraySize + vertexTexCoordIndex].Y();
				}
			}

			out_vertices.push_back(vertex);
		}
	}
}

// --------------------------------------------------------------------------------
void SceneManager::Triangulate(const std::vector<Vertex>& vertices, std::vector<Triangle>& out_triangles) const
{
	const uint32_t count = (uint32_t)vertices.size();

	for (uint32_t vertex = 0; vertex < count - 2u; vertex++)
	{
		Triangle triangle;

		triangle.m_vertices[0] = vertices[0u];
		triangle.m_vertices[1] = vertices[vertex + 1u];
		triangle.m_vertices[2] = vertices[vertex + 2u];

		out_triangles.push_back(triangle);
	}
}
