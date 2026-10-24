#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>


class UniformBuffer
{
public:
	UniformBuffer() = default;
	UniformBuffer(size_t size);
	~UniformBuffer();

	UniformBuffer(const UniformBuffer&) = delete;
	UniformBuffer& operator=(const UniformBuffer&) = delete;
	UniformBuffer(UniformBuffer&& other) noexcept;
	UniformBuffer& operator=(UniformBuffer&& other) noexcept;


	void SetData(const void* data, size_t size, size_t offset = 0);
	void BindToPoint(unsigned int bindingPoint, size_t offset = 0, size_t size = 0);

	inline int GetID() const { return m_RendererID; }

private:
	unsigned int m_RendererID{};
	size_t m_Size{};

};

