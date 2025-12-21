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

	std::vector<Triangle4> triangle4s;

	// Tri4  implementation
	void CreateTriangles3(const std::string& fileName, const float scaleFactor);
	void ParseAttributes(const std::string& line, const float scaleFactor, std::vector<float>& positionsX, std::vector<float>& positionsY,
		std::vector<float>& positionsZ, std::vector<float>& out_faceX, std::vector<float>& out_faceY, std::vector<float>& out_faceZ);
	void Triangulate(const std::vector<float>& faceValuesX, const std::vector<float>& faceValuesY, const std::vector<float>& faceValuesZ, 
		std::vector<float>& out_triangulatedPosX, std::vector<float>& out_triangulatedPosY, std::vector<float>& out_triangulatedPosZ);
	void CalculateSceneCenter(const std::vector<float>& positionsX, const std::vector<float>& positionsY, const std::vector<float>& positionsZ);
	void CreateTriangle4s(std::vector<float>& triangulatedPosX, std::vector<float> triangulatedPosY, std::vector<float> triangulatedPosZ);

	// Scalar with faces implementation
	void CreateTriangles(const std::string& fileName, const float scaleFactor, std::vector<Triangle>& out_triangles);
	void ParseAttributesWithFaces(const std::string& line, const float scaleFactor, std::vector<Vertex>& vertices);
	Vector3 CalculateSceneCenterWithFaces();
	void TriangulateWithFaces(const std::vector<Vertex>& vertices, std::vector<Triangle>& faces);

	void CreateTriangle4s(std::vector<Triangle> triangles, std::vector<Triangle4>& out_triangle4s) const;

	std::vector<Vector3> m_positions;
	std::vector<Vertex> m_vertices;


	Vector3 m_center;
};