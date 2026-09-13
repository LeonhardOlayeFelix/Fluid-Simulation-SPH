#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <array>

class IndexBuffer
{
private:
	unsigned int m_RendererId{};
	unsigned int m_Count{};
public:
	IndexBuffer() = default;
	IndexBuffer(const unsigned int* data, unsigned int count);
	template<size_t N>
	IndexBuffer(const std::array<unsigned int, N>& data) : IndexBuffer(data.data(), N){};
	IndexBuffer(const std::vector<unsigned int>& data) : IndexBuffer(data.data(), data.size()) {};

	void Bind() const;
	void Unbind() const;

	~IndexBuffer();
	IndexBuffer(const IndexBuffer&) = delete;
	IndexBuffer& operator=(const IndexBuffer&) = delete;
	IndexBuffer(IndexBuffer&& other) noexcept;
	IndexBuffer& operator=(IndexBuffer&& other) noexcept;

	inline unsigned int GetCount() const { return m_Count; }
};

