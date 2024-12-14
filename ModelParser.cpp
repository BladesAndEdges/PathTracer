#include "ModelParser.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <assert.h>

// --------------------------------------------------------------------------------
ModelParser::ModelParser()
{
}

// --------------------------------------------------------------------------------
void ModelParser::ParseFile(const char* objSourceFile, const float scaleFactor)
{
	assert(objSourceFile != nullptr);

	// For SSE
	CreateFaces(objSourceFile, scaleFactor);
	CalculateSceneCenter();

	//For scalar
	CreateFacesWithFaces(objSourceFile, scaleFactor);

	Vector3 center = CalculateSceneCenterWithFaces();
}

// --------------------------------------------------------------------------------
Vector3 ModelParser::GetCenter() const
{
	return m_center;
}

// --------------------------------------------------------------------------------
std::vector<float> ModelParser::GetPositionsX() const
{
	return m_positionsX;
}

// --------------------------------------------------------------------------------
std::vector<float> ModelParser::GetPositionsY() const
{
	return m_positionsY;
}

// --------------------------------------------------------------------------------
std::vector<float> ModelParser::GetPositionsZ() const
{
	return m_positionsZ;
}

// --------------------------------------------------------------------------------
void ModelParser::CreateFaces(const std::string& fileName, const float scaleFactor)
{
	std::ifstream ifs(fileName);

	if (!ifs.is_open())
	{
		throw std::exception("Could not read Mesh Data!");
	}

	std::string prefix;

	std::vector<float> positionsX;
	std::vector<float> positionsY;
	std::vector<float> positionsZ;

	while (ifs >> prefix)
	{
		// For SSE
		if (prefix == "v")
		{
			float x, y, z;
			ifs >> x >> y >> z;

			positionsX.push_back(x);
			positionsY.push_back(y);
			positionsZ.push_back(z);
		}

		// This is done per line, so the total number of faces to compute the currentMesh needs to be re-iterated
		// I will need to consider doing a single Vertex array, which would make the levels of indirection here substantially easier
		if (prefix == "f")
		{
			std::string line;
			std::getline(ifs, line);

			std::vector<float> faceX;
			std::vector<float> faceY;
			std::vector<float> faceZ;

			ParseAttributes(line, scaleFactor, positionsX, positionsY, positionsZ, faceX, faceY, faceZ);
			Triangulate(faceX, faceY, faceZ);
		}
	}
}

// --------------------------------------------------------------------------------
void ModelParser::CalculateSceneCenter()
{
	// For SSE
	float minX = m_positionsX[0u];
	float maxX = minX;

	float minY = m_positionsY[0u];
	float maxY = minY;

	float minZ = m_positionsZ[0u];
	float maxZ = minZ;

	for (uint32_t position = 1u; position < m_positionsX.size(); position++)
	{
		if (m_positionsX[position] < minX)
		{
			minX = m_positionsX[position];
		}

		if (m_positionsX[position] > maxX)
		{
			maxX = m_positionsX[position];
		}

		if (m_positionsY[position] < minX)
		{
			minX = m_positionsY[position];
		}

		if (m_positionsY[position] > maxX)
		{
			maxX = m_positionsY[position];
		}

		if (m_positionsZ[position] < minX)
		{
			minX = m_positionsZ[position];
		}

		if (m_positionsZ[position] > maxX)
		{
			maxX = m_positionsZ[position];
		}
	}

	m_center.SetX((minX + maxX) / 2.0f);
	m_center.SetY((minY + maxY) / 2.0f);
	m_center.SetZ((minZ + maxZ) / 2.0f);

	// For scalar
}

// --------------------------------------------------------------------------------
void ModelParser::ParseAttributes(const std::string& line, const float scaleFactor, std::vector<float>& positionsX, std::vector<float>& positionsY,
	std::vector<float>& positionsZ, std::vector<float>& out_faceX, std::vector<float>& out_faceY, std::vector<float>& out_faceZ)
{
	std::vector<Vector3> vertices;
	const char whitespace = ' ';

	std::istringstream lineStringStream(line);
	std::string vertexData;

	while (lineStringStream.good())
	{
		std::getline(lineStringStream, vertexData, whitespace);

		if (vertexData != "")
		{
			std::string backslash = "/";

			const size_t firstBackSlashIndex = vertexData.find(backslash);

			const std::string vertexPositionIndex = vertexData.substr(0, firstBackSlashIndex);
			const std::string emptyString = "";

			if (vertexPositionIndex != emptyString)
			{
				int vertexPositionId = stoi(vertexPositionIndex, nullptr);

				if (vertexPositionId >= 1)
				{
					out_faceX.push_back(scaleFactor * positionsX[vertexPositionId - 1u]);
					out_faceY.push_back(scaleFactor * positionsY[vertexPositionId - 1u]);
					out_faceZ.push_back(scaleFactor * positionsZ[vertexPositionId - 1u]);
				}
				else
				{
					const uint32_t numPositions = (uint32_t)positionsX.size();

					out_faceX.push_back(scaleFactor * positionsX[numPositions + vertexPositionId]);
					out_faceY.push_back(scaleFactor * positionsY[numPositions + vertexPositionId]);
					out_faceZ.push_back(scaleFactor * positionsZ[numPositions + vertexPositionId]);
				}
			}
		}
	}
}

// --------------------------------------------------------------------------------
void ModelParser::Triangulate(const std::vector<float>& faceX, const std::vector<float>& faceY, const std::vector<float>& faceZ)
{
	// For SSE
	const uint32_t vertexCount = (uint32_t)faceX.size();

	for (uint32_t triangle = 0u; triangle < vertexCount - 2u; triangle++)
	{
		m_positionsX.push_back(faceX[0u]);
		m_positionsY.push_back(faceY[0u]);
		m_positionsZ.push_back(faceZ[0u]);

		m_positionsX.push_back(faceX[triangle + 1u]);
		m_positionsY.push_back(faceY[triangle + 1u]);
		m_positionsZ.push_back(faceZ[triangle + 1u]);

		m_positionsX.push_back(faceX[triangle + 2u]);
		m_positionsY.push_back(faceY[triangle + 2u]);
		m_positionsZ.push_back(faceZ[triangle + 2u]);
	}
}
// --------------------------------------------------------------------------------
void ModelParser::CreateFacesWithFaces(const std::string& fileName, const float scaleFactor)
{
	std::ifstream ifs(fileName);

	if (!ifs.is_open())
	{
		throw std::exception("Could not read Mesh Data!");
	}

	std::string prefix;

	while (ifs >> prefix)
	{
		if (prefix == "v")
		{
			float x;
			float y;
			float z;

			ifs >> x >> y >> z;

			Vector3 position(x, y, z);
			Vertex v(x, y, z);
			m_vertices.push_back(v);
			m_positions.push_back(position);
		}

		// This is done per line, so the total number of faces to compute the currentMesh needs to be re-iterated
		// I will need to consider doing a single Vertex array, which would make the levels of indirection here substantially easier
		if (prefix == "f")
		{
			std::string line;
			std::getline(ifs, line);

			std::vector<Vertex> faceVertices;
			ParseAttributesWithFaces(line, scaleFactor, faceVertices);
			std::vector<Face> faces;
			TriangulateWithFaces(faceVertices, faces);

			m_faces.insert(std::end(m_faces), std::begin(faces), std::end(faces));
		}
	}
}

// --------------------------------------------------------------------------------
void ModelParser::ParseAttributesWithFaces(const std::string& line, const float scaleFactor, std::vector<Vertex>& vertices)
{
	const char whitespace = ' ';

	std::istringstream lineStringStream(line);
	std::string vertexData;

	while (lineStringStream.good())
	{
		std::getline(lineStringStream, vertexData, whitespace);

		if (vertexData != "")
		{
			std::string backslash = "/";

			Vertex vertex;

			const size_t firstBackSlashIndex = vertexData.find(backslash);
			//const size_t secondBackSlashIndex = vertexData.find(backslash, firstBackSlashIndex + 1);


			const std::string vertexPositionIndex = vertexData.substr(0, firstBackSlashIndex);

			// Figure out what happens if a value is not supplied
			Vector3 position(0.0f, 0.0f, 0.0f);

			const std::string emptyString = "";

			if (vertexPositionIndex != emptyString)
			{
				// You might be able to use the pointer to your advantage, do test it out
				// You can use this to track how far into the substring you are currently, and just simply 
				//continue extracting until this pointer is the end of the string

				int vertexPositionId = stoi(vertexPositionIndex, nullptr);

				if (vertexPositionId >= 1)
				{
					vertex.m_position[0] = m_positions[vertexPositionId - 1].X() * scaleFactor;
					vertex.m_position[1] = m_positions[vertexPositionId - 1].Y() * scaleFactor;
					vertex.m_position[2] = m_positions[vertexPositionId - 1].Z() * scaleFactor;
				}
				else
				{
					unsigned int arraySize = (unsigned int)m_positions.size();

					vertex.m_position[0] = m_positions[arraySize + vertexPositionId].X() * scaleFactor;
					vertex.m_position[1] = m_positions[arraySize + vertexPositionId].Y() * scaleFactor;
					vertex.m_position[2] = m_positions[arraySize + vertexPositionId].Z() * scaleFactor;
				}

				// Code for centering the camera position
			}

			vertices.push_back(vertex);
		}
	}
}

// --------------------------------------------------------------------------------
void ModelParser::TriangulateWithFaces(const std::vector<Vertex>& vertices, std::vector<Face>& faces)
{
	const int vertexCount = (int)vertices.size();

	for (int triangle = 0; triangle < vertexCount - 2; triangle++)
	{
		Face face;

		face.m_faceVertices[0] = vertices[0];
		face.m_faceVertices[1] = vertices[triangle + 1];
		face.m_faceVertices[2] = vertices[triangle + 2];

		faces.push_back(face);
	}
}

// --------------------------------------------------------------------------------
Vector3 ModelParser::CalculateSceneCenterWithFaces()
{
	float minX = m_vertices[0].m_position[0];
	float maxX = minX;

	float minY = m_vertices[0].m_position[1];
	float maxY = minY;

	float minZ = m_vertices[0].m_position[2];
	float maxZ = minZ;

	// Start from 1 since the initial vertex is used as a starting value
	for (unsigned int vertex = 1; vertex < m_vertices.size(); vertex++)
	{
		const float vertexX = m_vertices[vertex].m_position[0];
		const float vertexY = m_vertices[vertex].m_position[1];
		const float vertexZ = m_vertices[vertex].m_position[2];

		if (vertexX < minX)
		{
			minX = vertexX;
		}

		if (vertexX > maxX)
		{
			maxX = vertexX;
		}

		if (vertexY < minY)
		{
			minY = vertexY;
		}

		if (vertexY > maxY)
		{
			maxY = vertexY;
		}

		if (vertexZ < minZ)
		{
			minZ = vertexZ;
		}

		if (vertexZ > maxZ)
		{
			maxZ = vertexZ;
		}
	}

	const float halfwayX = (minX + maxX) / 2.0f;
	const float halfwayY = (minY + maxY) / 2.0f;
	const float halfWayZ = (minZ + maxZ) / 2.0f;

	return Vector3(halfwayX, halfwayY, halfWayZ);
}
