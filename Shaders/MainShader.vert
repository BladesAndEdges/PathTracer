#version 460 core

// --------------------------------------------------------------------------------
vec4 generateTriangleClipspaceCoordinate(uint vertexId)
{		
	vec4 clipSpaceCoord;
	
	// Do I need this as 1.0 or glDisable(GL_DEPTH) and use 0
	clipSpaceCoord.x = (float(vertexId / 2) * 4.0f) - 1.0f;
	clipSpaceCoord.y = (float(vertexId % 2) * 4.0f) - 1.0f;
	clipSpaceCoord.z = 0.0f;
	clipSpaceCoord.w = 1.0f;
	
	return clipSpaceCoord;
};

// --------------------------------------------------------------------------------
vec2 generateTriangleTextureCoordinates(uint vertexId)
{
	vec2 textureCoordinate;
	
	textureCoordinate.x = float(vertexId / 2) * 2.0f;
	textureCoordinate.y = 1.0f - (float(vertexId % 2) * 2.0f);
	
	return textureCoordinate;
};

out vec2 v2f_textureCoordinate;

// --------------------------------------------------------------------------------
void main()
{
	// To accomodate for CCW rendering, and avoid the triangle being culled
	const uint c_correctedVertexId = 2  - gl_VertexID;
	
	gl_Position = generateTriangleClipspaceCoordinate(c_correctedVertexId);
	v2f_textureCoordinate = generateTriangleTextureCoordinates(c_correctedVertexId);
}