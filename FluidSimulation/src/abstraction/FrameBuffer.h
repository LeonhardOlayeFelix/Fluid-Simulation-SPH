#pragma once
#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <variant>
#include <vector>
#include "Texture.h"
#include "RenderBuffer.h"
#include "CubeMap.h"

enum class AttachmentTarget {
	Color, Depth, Stencil, DepthStencil
};
enum class AttachmentStorage {
	Texture, RenderBuffer, CubeMap
};

class FrameBuffer
{
private:
	unsigned int m_RendererId{};
	int m_Width{};
	int m_Height{};
	int m_MSAASamples{};

	struct Attachment {
		AttachmentTarget target;
		int colorIndex;
		std::variant<Texture, RenderBuffer, CubeMap> storage;
	};

	std::vector<Attachment> m_Attachments{};
	std::vector<GLenum> m_DrawBuffers{};

public:
	FrameBuffer() = default;
	FrameBuffer(int width, int height, int samples = 1);

	void Bind() const;
	void Unbind() const;

	void AddAttachment(AttachmentTarget target, AttachmentStorage storage, int colorIndex = 0);
	bool Validate();
	void Blit(const FrameBuffer& target) const;
	void MarkAsNoColorBuffer();
	const Texture& GetColorTexture(int colorIndex = 0) const;
	const Texture& GetDepthTexture() const;
	const CubeMap& GetDepthCubeMap() const;


	~FrameBuffer();
	FrameBuffer(const FrameBuffer&) = delete;
	FrameBuffer& operator=(const FrameBuffer&) = delete;
	FrameBuffer(FrameBuffer&& other) noexcept;
	FrameBuffer& operator=(FrameBuffer&& other) noexcept;


	inline unsigned int GetId() const { return m_RendererId; };
	inline unsigned int GetWidth() const { return m_Width; };
	inline unsigned int GetHeight() const { return m_Height; };

};

