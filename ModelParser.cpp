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
	CreateTriangles(objSourceFile, scaleFactor);

	// For scalar, with SOAs
	CreateTriangles2(objSourceFile, scaleFactor);

	// For scalar, with faces
	CreateTriangles3(objSourceFile, scaleFactor);
}

// --------------------------------------------------------------------------------
Vector3 ModelParser::GetCenter() const
{
	return m_center;
}

// --------------------------------------------------------------------------------
std::vector<Triangle4> ModelParser::GetTriangle4Data() const
{
	return m_triangle4s;
}

std::vector<float> ModelParser::GetPositionsX() const
{
	return m_positionsX;
}

std::vector<float> ModelParser::GetPositionsY() const
{
	return m_positionsY;
}

std::vector<float> ModelParser::GetPositionsZ() const
{
	return m_positionsZ;
}

// --------------------------------------------------------------------------------
std::vector<Triangle> ModelParser::GetFaces() const
{
	return m_faces;
}

// --------------------------------------------------------------------------------
void ModelParser::CreateTriangles(const std::string& fileName, const float scaleFactor)
{
	std::ifstream ifs(fileName);

	if (!ifs.is_open())
	{
		throw std::exception("Could not read Mesh Data!");
	}

	std::string prefix;

	std::vector<float> readPositionsX;
	std::vector<float> readPositionsY;
	std::vector<float> readPositionsZ;

	std::vector<float> triangulatedPositionsX;
	std::vector<float> triangulatedPositionsY;
	std::vector<float> triangulatedPositionsZ;

	while (ifs >> prefix)
	{
		// For SSE
		if (prefix == "v")
		{
			float x, y, z;
			ifs >> x >> y >> z;

			readPositionsX.push_back(x);
			readPositionsY.push_back(y);
			readPositionsZ.push_back(z);
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

			ParseAttributes(line, scaleFactor, readPositionsX, readPositionsY, readPositionsZ, faceX, faceY, faceZ);
			Triangulate(faceX, faceY, faceZ, triangulatedPositionsX, triangulatedPositionsY, triangulatedPositionsZ);
		}
	}

	CalculateSceneCenter(readPositionsX, readPositionsY, readPositionsZ);
	CreateTriangle4s(triangulatedPositionsX, triangulatedPositionsY, triangulatedPositionsZ);
}

// --------------------------------------------------------------------------------
void ModelParser::Triangulate(const std::vector<float>& faceValuesX, const std::vector<float>& faceValuesY, const std::vector<float>& faceValuesZ,
	std::vector<float>& out_triangulatedPosX, std::vector<float>& out_triangulatedPosY, std::vector<float>& out_triangulatedPosZ)
{
	for (uint32_t triangle = 0u; triangle < faceValuesX.size() - 2u; triangle++)
	{
		out_triangulatedPosX.push_back(faceValuesX[0u]);
		out_triangulatedPosY.push_back(faceValuesY[0u]);
		out_triangulatedPosZ.push_back(faceValuesZ[0u]);

		out_triangulatedPosX.push_back(faceValuesX[triangle + 1u]);
		out_triangulatedPosY.push_back(faceValuesY[triangle + 1u]);
		out_triangulatedPosZ.push_back(faceValuesZ[triangle + 1u]);

		out_triangulatedPosX.push_back(faceValuesX[triangle + 2u]);
		out_triangulatedPosY.push_back(faceValuesY[triangle + 2u]);
		out_triangulatedPosZ.push_back(faceValuesZ[triangle + 2u]);
	}
}

// --------------------------------------------------------------------------------
void ModelParser::CalculateSceneCenter(const std::vector<float>& positionsX, const std::vector<float>& positionsY, const std::vector<float>& positionsZ)
{
	// For SSE
	float minX = positionsX[0u];
	float maxX = minX;

	float minY = positionsY[0u];
	float maxY = minY;

	float minZ = positionsZ[0u];
	float maxZ = minZ;

	for (uint32_t position = 1u; position < positionsX.size(); position++)
	{
		if (positionsX[position] < minX)
		{
			minX = positionsX[position];
		}

		if (positionsX[position] > maxX)
		{
			maxX = positionsX[position];
		}

		if (positionsY[position] < minY)
		{
			minY = positionsY[position];
		}

		if (positionsY[position] > maxY)
		{
			maxY = positionsY[position];
		}

		if (positionsZ[position] < minZ)
		{
			minZ = positionsZ[position];
		}

		if (positionsZ[position] > maxZ)
		{
			maxZ = positionsZ[position];
		}
	}

	m_center.SetX((minX + maxX) / 2.0f);
	m_center.SetY((minY + maxY) / 2.0f);
	m_center.SetZ((minZ + maxZ) / 2.0f);
}

// --------------------------------------------------------------------------------
void ModelParser::CreateTriangle4s(std::vector<float>& triangulatedPosX, std::vector<float> triangulatedPosY, std::vector<float> triangulatedPosZ)
{
	//Make sure the amount of triangles is a multiple of 4
	const uint32_t numTriangles = (uint32_t)triangulatedPosX.size() / 3u;
	if ((numTriangles % 4u) != 0u)
	{
		const uint32_t c_numToPad = 4u - numTriangles % 4u;

		for (uint32_t padding = 0u; padding < c_numToPad; padding++)
		{
			for (uint32_t vertex = 0u; vertex < 3u; vertex++)
			{
				triangulatedPosX.push_back(0.0f);
				triangulatedPosY.push_back(0.0f);
				triangulatedPosZ.push_back(0.0f);
			}
		}
	}

	// Create SSE data
	const uint32_t updatedNumTriangles = (uint32_t)triangulatedPosX.size() / 3u;
	for (uint32_t triangle = 0u; triangle < updatedNumTriangles; triangle += 4u)
	{
		Triangle4 tri4;

		const uint32_t tri0Offset = triangle * 3u;
		const uint32_t tri1Offset = (triangle + 1u) * 3u;
		const uint32_t tri2Offset = (triangle + 2u) * 3u;
		const uint32_t tri3Offset = (triangle + 3u) * 3u;

		//Triangle 0
		tri4.m_v0X.push_back(triangulatedPosX[tri0Offset]);
		tri4.m_v0Y.push_back(triangulatedPosY[tri0Offset]);
		tri4.m_v0Z.push_back(triangulatedPosZ[tri0Offset]);

		tri4.m_edge1X.push_back(triangulatedPosX[tri0Offset + 1u] - triangulatedPosX[tri0Offset]);
		tri4.m_edge1Y.push_back(triangulatedPosY[tri0Offset + 1u] - triangulatedPosY[tri0Offset]);
		tri4.m_edge1Z.push_back(triangulatedPosZ[tri0Offset + 1u] - triangulatedPosZ[tri0Offset]);

		tri4.m_edge2X.push_back(triangulatedPosX[tri0Offset + 2u] - triangulatedPosX[tri0Offset]);
		tri4.m_edge2Y.push_back(triangulatedPosY[tri0Offset + 2u] - triangulatedPosY[tri0Offset]);
		tri4.m_edge2Z.push_back(triangulatedPosZ[tri0Offset + 2u] - triangulatedPosZ[tri0Offset]);

		//Triangle 1
		tri4.m_v0X.push_back(triangulatedPosX[tri1Offset]);
		tri4.m_v0Y.push_back(triangulatedPosY[tri1Offset]);
		tri4.m_v0Z.push_back(triangulatedPosZ[tri1Offset]);

		tri4.m_edge1X.push_back(triangulatedPosX[tri1Offset + 1u] - triangulatedPosX[tri1Offset]);
		tri4.m_edge1Y.push_back(triangulatedPosY[tri1Offset + 1u] - triangulatedPosY[tri1Offset]);
		tri4.m_edge1Z.push_back(triangulatedPosZ[tri1Offset + 1u] - triangulatedPosZ[tri1Offset]);

		tri4.m_edge2X.push_back(triangulatedPosX[tri1Offset + 2u] - triangulatedPosX[tri1Offset]);
		tri4.m_edge2Y.push_back(triangulatedPosY[tri1Offset + 2u] - triangulatedPosY[tri1Offset]);
		tri4.m_edge2Z.push_back(triangulatedPosZ[tri1Offset + 2u] - triangulatedPosZ[tri1Offset]);

		//Triangle 2
		tri4.m_v0X.push_back(triangulatedPosX[tri2Offset]);
		tri4.m_v0Y.push_back(triangulatedPosY[tri2Offset]);
		tri4.m_v0Z.push_back(triangulatedPosZ[tri2Offset]);

		tri4.m_edge1X.push_back(triangulatedPosX[tri2Offset + 1u] - triangulatedPosX[tri2Offset]);
		tri4.m_edge1Y.push_back(triangulatedPosY[tri2Offset + 1u] - triangulatedPosY[tri2Offset]);
		tri4.m_edge1Z.push_back(triangulatedPosZ[tri2Offset + 1u] - triangulatedPosZ[tri2Offset]);

		tri4.m_edge2X.push_back(triangulatedPosX[tri2Offset + 2u] - triangulatedPosX[tri2Offset]);
		tri4.m_edge2Y.push_back(triangulatedPosY[tri2Offset + 2u] - triangulatedPosY[tri2Offset]);
		tri4.m_edge2Z.push_back(triangulatedPosZ[tri2Offset + 2u] - triangulatedPosZ[tri2Offset]);

		//Triangle 3
		tri4.m_v0X.push_back(triangulatedPosX[tri3Offset]);
		tri4.m_v0Y.push_back(triangulatedPosY[tri3Offset]);
		tri4.m_v0Z.push_back(triangulatedPosZ[tri3Offset]);

		tri4.m_edge1X.push_back(triangulatedPosX[tri3Offset + 1u] - triangulatedPosX[tri3Offset]);
		tri4.m_edge1Y.push_back(triangulatedPosY[tri3Offset + 1u] - triangulatedPosY[tri3Offset]);
		tri4.m_edge1Z.push_back(triangulatedPosZ[tri3Offset + 1u] - triangulatedPosZ[tri3Offset]);

		tri4.m_edge2X.push_back(triangulatedPosX[tri3Offset + 2u] - triangulatedPosX[tri3Offset]);
		tri4.m_edge2Y.push_back(triangulatedPosY[tri3Offset + 2u] - triangulatedPosY[tri3Offset]);
		tri4.m_edge2Z.push_back(triangulatedPosZ[tri3Offset + 2u] - triangulatedPosZ[tri3Offset]);

		m_triangle4s.push_back(tri4);
	}

}

void ModelParser::CreateTriangles2(const std::string& fileName, const float scaleFactor)
{
	std::ifstream ifs(fileName);

	if (!ifs.is_open())
	{
		throw std::exception("Could not read Mesh Data!");
	}

	std::string prefix;

	std::vector<float> readPositionsX;
	std::vector<float> readPositionsY;
	std::vector<float> readPositionsZ;

	while (ifs >> prefix)
	{
		// For SSE
		if (prefix == "v")
		{
			float x, y, z;
			ifs >> x >> y >> z;

			readPositionsX.push_back(x);
			readPositionsY.push_back(y);
			readPositionsZ.push_back(z);
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

			ParseAttributes(line, scaleFactor, readPositionsX, readPositionsY, readPositionsZ, faceX, faceY, faceZ);
			Triangulate2(faceX, faceY, faceZ);
		}
	}
}

void ModelParser::Triangulate2(const std::vector<float>& faceX, const std::vector<float>& faceY, const std::vector<float>& faceZ)
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
void ModelParser::ParseAttributes(const std::string& line, const float scaleFactor, std::vector<float>& positionsX, std::vector<float>& positionsY,
	std::vector<float>& positionsZ, std::vector<float>& out_faceX, std::vector<float>& out_faceY, std::vector<float>& out_faceZ)
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
void ModelParser::CreateTriangles3(const std::string& fileName, const float scaleFactor)
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
			std::vector<Triangle> faces;
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
void ModelParser::TriangulateWithFaces(const std::vector<Vertex>& vertices, std::vector<Triangle>& faces)
{
	const int vertexCount = (int)vertices.size();

	for (int triangle = 0; triangle < vertexCount - 2; triangle++)
	{
		Triangle face;

		face.m_vertices[0] = vertices[0];
		face.m_vertices[1] = vertices[triangle + 1];
		face.m_vertices[2] = vertices[triangle + 2];

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
