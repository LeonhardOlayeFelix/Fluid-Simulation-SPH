#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class RenderBuffer
{
private:
    unsigned int m_RendererId{};
    int m_Width{}, m_Height{};
public:
	RenderBuffer() = default;
	RenderBuffer(int width, int height, GLenum internalFormat = GL_DEPTH24_STENCIL8, int samples = 1);

    void Bind() const;
    void Unbind() const;

    ~RenderBuffer();
    RenderBuffer(const RenderBuffer&) = delete;
    RenderBuffer& operator=(const RenderBuffer&) = delete;
    RenderBuffer(RenderBuffer&& other) noexcept;
    RenderBuffer& operator=(RenderBuffer&& other) noexcept;

    unsigned int GetID() const { return m_RendererId; }
};

