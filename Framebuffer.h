#pragma once

#include<cstdint>

class Framebuffer
{
public:

	Framebuffer(uint32_t width, uint32_t height, uint32_t channels);
	Framebuffer(const Framebuffer&) = delete;
	Framebuffer& operator=(const Framebuffer&) = delete;

	void Use() const;
	void Update() const;

	~Framebuffer();

	uint32_t GetWidth() const;
	uint32_t GetHeight() const;
	uint32_t GetNumChannels() const;
	uint8_t* GetDataPtr();

private:

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_numChannels;

	uint32_t m_texture;
	uint8_t* m_data;
};

