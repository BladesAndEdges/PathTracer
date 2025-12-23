#include "ModelParser.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <assert.h>
#include "Triangle.h"
#include "Triangle4.h"

// --------------------------------------------------------------------------------
ModelParser::ModelParser()
{
}

// --------------------------------------------------------------------------------
void ModelParser::ParseFile(const char* objSourceFile, const float scaleFactor, std::vector<Triangle>& out_triangles, std::vector<Triangle4>& out_triangle4s)
{
	assert(objSourceFile != nullptr);
	assert(scaleFactor > 0.0f);
	assert(out_triangles.size() == 0);
	assert(out_triangle4s.size() == 0);

	CreateTriangles(objSourceFile, scaleFactor, out_triangles);
	CreateTriangle4s(out_triangles, out_triangle4s);

	assert(out_triangles.size() > 0);
	assert(out_triangle4s.size() > 0);
}

// --------------------------------------------------------------------------------
void ModelParser::CreateTriangles(const std::string& fileName, const float scaleFactor, std::vector<Triangle>& out_triangles)
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
			ParseAttributes(line, scaleFactor, faceVertices);
			std::vector<Triangle> triangles;
			Triangulate(faceVertices, triangles);

			for (uint32_t i = 0u; i < triangles.size(); i++)
			{
				triangles[i].m_edge1.SetX(triangles[i].m_vertices[1u].m_position[0u] - triangles[i].m_vertices[0u].m_position[0u]);
				triangles[i].m_edge1.SetY(triangles[i].m_vertices[1u].m_position[1u] - triangles[i].m_vertices[0u].m_position[1u]);
				triangles[i].m_edge1.SetZ(triangles[i].m_vertices[1u].m_position[2u] - triangles[i].m_vertices[0u].m_position[2u]);

				triangles[i].m_edge2.SetX(triangles[i].m_vertices[2u].m_position[0u] - triangles[i].m_vertices[0u].m_position[0u]);
				triangles[i].m_edge2.SetY(triangles[i].m_vertices[2u].m_position[1u] - triangles[i].m_vertices[0u].m_position[1u]);
				triangles[i].m_edge2.SetZ(triangles[i].m_vertices[2u].m_position[2u] - triangles[i].m_vertices[0u].m_position[2u]);
			}

			out_triangles.insert(std::end(out_triangles), std::begin(triangles), std::end(triangles));
		}
	}
}

// --------------------------------------------------------------------------------
void ModelParser::ParseAttributes(const std::string& line, const float scaleFactor, std::vector<Vertex>& vertices)
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
void ModelParser::Triangulate(const std::vector<Vertex>& vertices, std::vector<Triangle>& faces)
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
void ModelParser::CreateTriangle4s(std::vector<Triangle> triangles, std::vector<Triangle4>& out_triangle4s) const
{
	// Pad to a multiple of 4, if needed
	const uint32_t remainder = (uint32_t)triangles.size() % 4u;
	if (remainder != 0u)
	{
		const uint32_t padCount = 4u - remainder;
		for (uint32_t padding = 0u; padding < padCount; padding++)
		{
			triangles.push_back(Triangle());
		}
	}

	Triangle4 triangle4;
	for (uint32_t index = 0u; index < triangles.size(); index += 4u)
	{
		// First triangle
		triangle4.m_v0X[0u] = triangles[index].m_vertices[0u].m_position[0u];
		triangle4.m_v0Y[0u] = triangles[index].m_vertices[0u].m_position[1u];
		triangle4.m_v0Z[0u] = triangles[index].m_vertices[0u].m_position[2u];

		triangle4.m_edge1X[0u] = triangles[index].m_vertices[1u].m_position[0u] - triangles[index].m_vertices[0u].m_position[0u];
		triangle4.m_edge1Y[0u] = triangles[index].m_vertices[1u].m_position[1u] - triangles[index].m_vertices[0u].m_position[1u];
		triangle4.m_edge1Z[0u] = triangles[index].m_vertices[1u].m_position[2u] - triangles[index].m_vertices[0u].m_position[2u];

		triangle4.m_edge2X[0u] = triangles[index].m_vertices[2u].m_position[0u] - triangles[index].m_vertices[0u].m_position[0u];
		triangle4.m_edge2Y[0u] = triangles[index].m_vertices[2u].m_position[1u] - triangles[index].m_vertices[0u].m_position[1u];
		triangle4.m_edge2Z[0u] = triangles[index].m_vertices[2u].m_position[2u] - triangles[index].m_vertices[0u].m_position[2u];

		// Second triangle
		triangle4.m_v0X[1u] = triangles[index + 1u].m_vertices[0u].m_position[0u];
		triangle4.m_v0Y[1u] = triangles[index + 1u].m_vertices[0u].m_position[1u];
		triangle4.m_v0Z[1u] = triangles[index + 1u].m_vertices[0u].m_position[2u];

		triangle4.m_edge1X[1u] = triangles[index + 1u].m_vertices[1u].m_position[0u] - triangles[index + 1u].m_vertices[0u].m_position[0u];
		triangle4.m_edge1Y[1u] = triangles[index + 1u].m_vertices[1u].m_position[1u] - triangles[index + 1u].m_vertices[0u].m_position[1u];
		triangle4.m_edge1Z[1u] = triangles[index + 1u].m_vertices[1u].m_position[2u] - triangles[index + 1u].m_vertices[0u].m_position[2u];

		triangle4.m_edge2X[1u] = triangles[index + 1u].m_vertices[2u].m_position[0u] - triangles[index + 1u].m_vertices[0u].m_position[0u];
		triangle4.m_edge2Y[1u] = triangles[index + 1u].m_vertices[2u].m_position[1u] - triangles[index + 1u].m_vertices[0u].m_position[1u];
		triangle4.m_edge2Z[1u] = triangles[index + 1u].m_vertices[2u].m_position[2u] - triangles[index + 1u].m_vertices[0u].m_position[2u];

		// Third triangle
		triangle4.m_v0X[2u] = triangles[index + 2u].m_vertices[0u].m_position[0u];
		triangle4.m_v0Y[2u] = triangles[index + 2u].m_vertices[0u].m_position[1u];
		triangle4.m_v0Z[2u] = triangles[index + 2u].m_vertices[0u].m_position[2u];

		triangle4.m_edge1X[2u] = triangles[index + 2u].m_vertices[1u].m_position[0u] - triangles[index + 2u].m_vertices[0u].m_position[0u];
		triangle4.m_edge1Y[2u] = triangles[index + 2u].m_vertices[1u].m_position[1u] - triangles[index + 2u].m_vertices[0u].m_position[1u];
		triangle4.m_edge1Z[2u] = triangles[index + 2u].m_vertices[1u].m_position[2u] - triangles[index + 2u].m_vertices[0u].m_position[2u];

		triangle4.m_edge2X[2u] = triangles[index + 2u].m_vertices[2u].m_position[0u] - triangles[index + 2u].m_vertices[0u].m_position[0u];
		triangle4.m_edge2Y[2u] = triangles[index + 2u].m_vertices[2u].m_position[1u] - triangles[index + 2u].m_vertices[0u].m_position[1u];
		triangle4.m_edge2Z[2u] = triangles[index + 2u].m_vertices[2u].m_position[2u] - triangles[index + 2u].m_vertices[0u].m_position[2u];

		// Fourth triangle
		triangle4.m_v0X[3u] = triangles[index + 3u].m_vertices[0u].m_position[0u];
		triangle4.m_v0Y[3u] = triangles[index + 3u].m_vertices[0u].m_position[1u];
		triangle4.m_v0Z[3u] = triangles[index + 3u].m_vertices[0u].m_position[2u];

		triangle4.m_edge1X[3u] = triangles[index + 3u].m_vertices[1u].m_position[0u] - triangles[index + 3u].m_vertices[0u].m_position[0u];
		triangle4.m_edge1Y[3u] = triangles[index + 3u].m_vertices[1u].m_position[1u] - triangles[index + 3u].m_vertices[0u].m_position[1u];
		triangle4.m_edge1Z[3u] = triangles[index + 3u].m_vertices[1u].m_position[2u] - triangles[index + 3u].m_vertices[0u].m_position[2u];

		triangle4.m_edge2X[3u] = triangles[index + 3u].m_vertices[2u].m_position[0u] - triangles[index + 3u].m_vertices[0u].m_position[0u];
		triangle4.m_edge2Y[3u] = triangles[index + 3u].m_vertices[2u].m_position[1u] - triangles[index + 3u].m_vertices[0u].m_position[1u];
		triangle4.m_edge2Z[3u] = triangles[index + 3u].m_vertices[2u].m_position[2u] - triangles[index + 3u].m_vertices[0u].m_position[2u];

		out_triangle4s.push_back(triangle4);
	}
}
