#pragma once

#include <string>
#include <vector>

class TextureManager
{
public:

	TextureManager();

	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;

	uint32_t Load(const char* textureName);

private:

	std::vector<std::string> m_textureNames;
	std::vector<const uint8_t*> m_data;
};

