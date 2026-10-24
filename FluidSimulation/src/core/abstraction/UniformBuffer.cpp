#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(size_t size) : m_Size(size)
{
	glCreateBuffers(1, &m_RendererID);
	glNamedBufferData(m_RendererID, size, NULL, GL_STATIC_DRAW);
}

UniformBuffer::~UniformBuffer()
{
	if (m_RendererID != 0)
		glDeleteBuffers(1, &m_RendererID);
}

UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
	: m_RendererID(other.m_RendererID), m_Size(other.m_Size)
{
	other.m_RendererID = 0;
	other.m_Size = 0;
}

UniformBuffer& UniformBuffer::operator=(UniformBuffer && other) noexcept
{
	if (this != &other) {
		if (m_RendererID != 0)
			glDeleteBuffers(1, &m_RendererID);
		m_RendererID = other.m_RendererID;
		m_Size = other.m_Size;
		other.m_RendererID = 0;
		other.m_Size = 0;
	}

	return *this;
}

void UniformBuffer::SetData(const void* data, size_t size, size_t offset)
{
	glNamedBufferSubData(m_RendererID, offset, size, data);
}

void UniformBuffer::BindToPoint(unsigned int bindingPoint, size_t offset, size_t size)
{
	glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, m_RendererID, offset, size == 0 ? m_Size : size);
}
