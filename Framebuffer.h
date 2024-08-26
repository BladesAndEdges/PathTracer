#pragma once

#include<cstdint>

class Framebuffer
{
public:

	Framebuffer(uint32_t width, uint32_t height, uint32_t channels);
	Framebuffer(const Framebuffer&) = delete;
	Framebuffer& operator=(const Framebuffer&) = delete;

	void use() const; // glBindTextureUnit
	void update() const; // glSubImage2D only

	~Framebuffer();

	uint32_t getWidth() const;
	uint32_t getHeight() const;
	uint32_t getChannels() const;
	uint8_t* getData();

private:

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_channels;

	uint32_t m_texture;
	uint8_t* m_data;
};

