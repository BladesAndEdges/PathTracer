#pragma once

#include <string>
#include <vector>

#include "Vector3.h"
#include "Vertex.h"

#include "Triangle4.h"

struct Triangle;

// --------------------------------------------------------------------------------
class ModelParser
{
public:

	ModelParser();

	void ParseFile(const char* objSourceFile, const float scaleFactor, std::vector<Triangle>& out_triangles, std::vector<Triangle4>& out_triangle4s);

	Vector3 GetCenter() const;

private:

	// Scalar with faces implementation
	void CreateTriangles(const std::string& fileName, const float scaleFactor, std::vector<Triangle>& out_triangles);
	void ParseAttributes(const std::string& line, const float scaleFactor, std::vector<Vertex>& vertices);
	void Triangulate(const std::vector<Vertex>& vertices, std::vector<Triangle>& faces);

	void CreateTriangle4s(std::vector<Triangle> triangles, std::vector<Triangle4>& out_triangle4s) const;

	std::vector<Vector3> m_positions;
	std::vector<Vertex> m_vertices;
};