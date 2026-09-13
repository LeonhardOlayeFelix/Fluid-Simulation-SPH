#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
class Texture
{
private:
	unsigned int m_RendererId{};
	mutable unsigned int m_LastBoundSlot{};
	std::string m_FilePath{};
	unsigned char* m_LocalBuffer{};
	int m_Width{}, m_Height{}, m_BPP{};

public:
	Texture() = default;
	Texture(const std::string path, GLenum internalFormat = GL_RGBA8);

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;
	static Texture CreateEmpty(int width, int height, GLenum internalFormat, int samples = 1);

	~Texture();
	Texture(Texture&& other) noexcept;
	Texture& operator=(Texture&& other) noexcept;
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	inline int GetWidth() { return m_Width; }
	inline int GetHeight() { return m_Height; }
	inline unsigned int GetId() const { return m_RendererId; }
	inline unsigned int GetLastBoundSlot() const { return m_LastBoundSlot; }

};

