#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <array>
#include <vector>
#include "Primitives.h"

class VertexBuffer
{
private:
	unsigned int m_RendererId{};
	unsigned int m_Size{};

public:
	VertexBuffer() = default;
	VertexBuffer(const void* data, size_t size);
	VertexBuffer(const Primitive& primitive) : VertexBuffer(primitive.positions) {};

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

	inline unsigned int GetSize() const { return m_Size; };
	inline unsigned int GetId() const { return m_RendererId; };
};

