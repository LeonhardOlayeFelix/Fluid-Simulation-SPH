#include "RenderBuffer.h"

RenderBuffer::RenderBuffer(int width, int height, GLenum internalFormat, int samples) : m_RendererId(0), m_Width(width), m_Height(height)
{
    glCreateRenderbuffers(1, &m_RendererId);

    if (samples > 1) 
        glNamedRenderbufferStorageMultisample(m_RendererId, samples, internalFormat, width, height);
    else 
        glNamedRenderbufferStorage(m_RendererId, internalFormat, width, height);
}

void RenderBuffer::Bind() const
{
    glBindRenderbuffer(GL_RENDERBUFFER, m_RendererId);
}

void RenderBuffer::Unbind() const
{
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

RenderBuffer::~RenderBuffer()
{
    if (m_RendererId != 0) glDeleteRenderbuffers(1, &m_RendererId);
}

RenderBuffer::RenderBuffer(RenderBuffer && other) noexcept
{
    m_RendererId = other.m_RendererId;
    m_Width = other.m_Width;
    m_Height = other.m_Height;

    other.m_RendererId = 0;
}

RenderBuffer& RenderBuffer::operator=(RenderBuffer && other) noexcept
{
    if (this == &other) return *this;

    if (m_RendererId != 0) glDeleteRenderbuffers(1, &m_RendererId);

    m_RendererId = other.m_RendererId;
    m_Width = other.m_Width;
    m_Height = other.m_Height;

    other.m_RendererId = 0;

    return *this;
}
