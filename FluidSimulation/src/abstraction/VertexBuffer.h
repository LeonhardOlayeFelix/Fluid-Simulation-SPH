#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <array>
#include <vector>

class VertexBuffer
{
private:
	unsigned int m_RendererId{};
	unsigned int m_Count{};

public:
	VertexBuffer() = default;
	VertexBuffer(const void* data, size_t size);

	template<size_t N>
	VertexBuffer(const std::array<float, N>& data) : VertexBuffer(data.data(), N * sizeof(float)) {};
	VertexBuffer(const std::vector<float>& data) : VertexBuffer(data.data(), data.size() * sizeof(float)) {};

	void Bind() const;
	void Unbind() const;

	~VertexBuffer();
	VertexBuffer(const VertexBuffer& other) = delete;
	VertexBuffer& operator=(const VertexBuffer& other) = delete;
	VertexBuffer(VertexBuffer&& other) noexcept;
	VertexBuffer& operator=(VertexBuffer&& other) noexcept;

	inline unsigned int GetCount() const { return m_Count; };
	inline unsigned int GetId() const { return m_RendererId; };
};

