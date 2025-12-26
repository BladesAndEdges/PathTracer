#pragma once

#include <string>
#include <vector>

#include "Vector3.h"
#include "Vertex.h"

struct Triangle;

// --------------------------------------------------------------------------------
class ModelParser
{
public:

	ModelParser();

	void ParseFile(const char* objSourceFile, const float scaleFactor, std::vector<Triangle>& out_triangles);

	Vector3 GetCenter() const;

private:

	// Scalar with faces implementation
	void CreateTriangles(const std::string& fileName, const float scaleFactor, std::vector<Triangle>& out_triangles);
	void ParseAttributes(const std::string& line, const float scaleFactor, std::vector<Vertex>& vertices);
	void Triangulate(const std::vector<Vertex>& vertices, std::vector<Triangle>& faces);

	std::vector<Vector3> m_positions;
	std::vector<Vertex> m_vertices;
};