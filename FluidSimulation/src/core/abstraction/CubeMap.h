#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <array>
#include <string>

enum CubemapFace { RIGHT, LEFT, TOP, BOTTOM, FRONT, BACK };

class CubeMap
{
private:
	unsigned int m_RendererId{};
	mutable unsigned int m_LastBoundSlot{};
	std::array<std::string, 6> m_FilePaths{};
	unsigned char* m_LocalBuffer{};
	int m_Width{}, m_Height{}, m_BPP{};

public:
	CubeMap() = default;
	CubeMap(const std::string& path);
	CubeMap(const std::array<std::string, 6>& paths);

	static CubeMap CreateEmpty(int width, int height, GLenum internalFormat = GL_DEPTH_COMPONENT);
	void Bind(unsigned int slot = 0) const;
	void Unbind(unsigned int slot) const;


	CubeMap(CubeMap&& other) noexcept;
	CubeMap& operator=(CubeMap&& other) noexcept;
	CubeMap(const CubeMap&) = delete;
	CubeMap& operator=(const CubeMap&) = delete;
	~CubeMap();

	inline int GetWidth() { return m_Width; }
	inline int GetHeight() { return m_Height; }
	inline unsigned int GetId() const { return m_RendererId; }
	inline unsigned int GetLastBoundSlot() const { return m_LastBoundSlot; };
};

