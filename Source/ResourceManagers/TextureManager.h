#pragma once

#include <string>
#include <vector>

struct Material;
class Vector3;

// --------------------------------------------------------------------------------
class TextureManager
{
public:

	TextureManager();

	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;

	uint32_t Load(const char* textureName);

	Vector3 BasicSample(const Material& material, const float u, const float v) const;

private:

	struct RGB
	{
		uint8_t red;
		uint8_t green;
		uint8_t blue;
	};

	struct Texture
	{
		uint32_t m_width;
		uint32_t m_height;
		const RGB* m_data;
	};

	std::vector<std::string> m_textureNames;
	std::vector<Texture> m_textures;
};

