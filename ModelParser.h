#pragma once

#include <string>
#include <vector>

#include "Face.h"
#include "Triangle4.h"
#include "Vector3.h"
#include "Vertex.h"


// --------------------------------------------------------------------------------
class ModelParser
{
public:

	ModelParser();

	void ParseFile(const char* objSourceFile, const float scaleFactor);

	Vector3 GetCenter() const;

	std::vector<Triangle4> GetTriangle4Data() const;

	std::vector<float> GetPositionsX() const;
	std::vector<float> GetPositionsY() const;
	std::vector<float> GetPositionsZ() const;

	std::vector<Face> GetFaces() const;

private:

	// Tri4  implementation
	void CreateTriangles(const std::string& fileName, const float scaleFactor);
	void ParseAttributes(const std::string& line, const float scaleFactor, std::vector<float>& positionsX, std::vector<float>& positionsY,
		std::vector<float>& positionsZ, std::vector<float>& out_faceX, std::vector<float>& out_faceY, std::vector<float>& out_faceZ);
	void Triangulate(const std::vector<float>& faceValuesX, const std::vector<float>& faceValuesY, const std::vector<float>& faceValuesZ, 
		std::vector<float>& out_triangulatedPosX, std::vector<float>& out_triangulatedPosY, std::vector<float>& out_triangulatedPosZ);
	void CalculateSceneCenter(const std::vector<float>& positionsX, const std::vector<float>& positionsY, const std::vector<float>& positionsZ);
	void CreateTriangle4s(std::vector<float>& triangulatedPosX, std::vector<float> triangulatedPosY, std::vector<float> triangulatedPosZ);

	std::vector<Triangle4> m_triangle4s;
	// Scalar without faces implementation
	void CreateTriangles2(const std::string& fileName, const float scaleFactor);
	void Triangulate2(const std::vector<float>& faceX, const std::vector<float>& faceY, const std::vector<float>& faceZ);

	std::vector<float> m_positionsX;
	std::vector<float> m_positionsY;
	std::vector<float> m_positionsZ;

	// Scalar with faces implementation
	void CreateTriangles3(const std::string& fileName, const float scaleFactor);
	void ParseAttributesWithFaces(const std::string& line, const float scaleFactor, std::vector<Vertex>& vertices);
	Vector3 CalculateSceneCenterWithFaces();
	void TriangulateWithFaces(const std::vector<Vertex>& vertices, std::vector<Face>& faces);

	std::vector<Vector3> m_positions;
	std::vector<Vertex> m_vertices;
	std::vector<Face> m_faces;

	Vector3 m_center;
};