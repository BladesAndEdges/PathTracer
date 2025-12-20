#pragma once

#include <string>
#include <vector>

#include "Triangle4.h"
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

	const std::vector<Triangle4>& GetTriangle4Data() const;

private:

	// Tri4  implementation
	void CreateTriangles3(const std::string& fileName, const float scaleFactor);
	void ParseAttributes(const std::string& line, const float scaleFactor, std::vector<float>& positionsX, std::vector<float>& positionsY,
		std::vector<float>& positionsZ, std::vector<float>& out_faceX, std::vector<float>& out_faceY, std::vector<float>& out_faceZ);
	void Triangulate(const std::vector<float>& faceValuesX, const std::vector<float>& faceValuesY, const std::vector<float>& faceValuesZ, 
		std::vector<float>& out_triangulatedPosX, std::vector<float>& out_triangulatedPosY, std::vector<float>& out_triangulatedPosZ);
	void CalculateSceneCenter(const std::vector<float>& positionsX, const std::vector<float>& positionsY, const std::vector<float>& positionsZ);
	void CreateTriangle4s(std::vector<float>& triangulatedPosX, std::vector<float> triangulatedPosY, std::vector<float> triangulatedPosZ);

	std::vector<Triangle4> m_triangle4s;

	// Scalar with faces implementation
	void CreateTriangles(const std::string& fileName, const float scaleFactor, std::vector<Triangle>& out_triangles);
	void ParseAttributesWithFaces(const std::string& line, const float scaleFactor, std::vector<Vertex>& vertices);
	Vector3 CalculateSceneCenterWithFaces();
	void TriangulateWithFaces(const std::vector<Vertex>& vertices, std::vector<Triangle>& faces);

	std::vector<Vector3> m_positions;
	std::vector<Vertex> m_vertices;


	Vector3 m_center;
};