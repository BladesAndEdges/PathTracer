#version 460 core

in vec2 v2f_textureCoordinate;
out vec3 o_finalColour;

layout(binding = 0) uniform sampler2D mainTextureSampler;

void main()
{
	o_finalColour = texture(mainTextureSampler, v2f_textureCoordinate).rgb;
}