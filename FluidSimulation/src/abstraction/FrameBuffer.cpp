#include "FrameBuffer.h"
#include "../debug/Log.h"

namespace {
	GLenum ToGLAttachmentPoint(AttachmentTarget target, int colorIndex)
	{
		switch (target)
		{
		case AttachmentTarget::Color:        return GL_COLOR_ATTACHMENT0 + colorIndex;
		case AttachmentTarget::Depth:        return GL_DEPTH_ATTACHMENT;
		case AttachmentTarget::Stencil:      return GL_STENCIL_ATTACHMENT;
		case AttachmentTarget::DepthStencil: return GL_DEPTH_STENCIL_ATTACHMENT;
		}
		return 0;
	}

	GLenum ToGLInternalFormat(AttachmentTarget target)
	{
		switch (target)
		{
		case AttachmentTarget::Color:        return GL_RGBA8;
		case AttachmentTarget::Depth:        return GL_DEPTH_COMPONENT24;
		case AttachmentTarget::Stencil:      return GL_STENCIL_INDEX8;
		case AttachmentTarget::DepthStencil: return GL_DEPTH24_STENCIL8;
		}
		return 0;
	}
}

FrameBuffer::FrameBuffer(int width, int height, int samples) : m_Width(width), m_Height(height), m_MSAASamples(samples)
{
    glGenFramebuffers(1, &m_RendererId);
}

void FrameBuffer::Bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);
}

void FrameBuffer::Unbind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::AddAttachment(AttachmentTarget target, AttachmentStorage storage, int colorIndex)
{
	Bind();

	GLenum attachPoint = ToGLAttachmentPoint(target, colorIndex);
	Attachment attachment{ target, colorIndex, {} };

	if (storage == AttachmentStorage::Texture)
	{
		Texture tex = Texture::CreateEmpty(m_Width, m_Height, ToGLInternalFormat(target), m_MSAASamples);
		glNamedFramebufferTexture(m_RendererId, attachPoint, tex.GetId(), 0);
		attachment.storage = std::move(tex);
	}
	else if (storage == AttachmentStorage::RenderBuffer)
	{
		RenderBuffer rbo(m_Width, m_Height, ToGLInternalFormat(target), m_MSAASamples);
		glNamedFramebufferRenderbuffer(m_RendererId, attachPoint, GL_RENDERBUFFER, rbo.GetID());
		attachment.storage = std::move(rbo);
	}
	else if (storage == AttachmentStorage::CubeMap)
	{
		CubeMap cubeMap = CubeMap::CreateEmpty(m_Width, m_Height, GL_DEPTH_COMPONENT);
		glNamedFramebufferTexture(m_RendererId, attachPoint, cubeMap.GetId(), 0);
		attachment.storage = std::move(cubeMap);
	}

	if (target == AttachmentTarget::Color)
	{
		m_DrawBuffers.push_back(attachPoint);
		glDrawBuffers(static_cast<GLsizei>(m_DrawBuffers.size()), m_DrawBuffers.data());
	}

	m_Attachments.push_back(std::move(attachment));

	Unbind();

}

bool FrameBuffer::Validate()
{
	Bind();
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	bool isComplete = (status == GL_FRAMEBUFFER_COMPLETE);
	if (!isComplete)
		SPHWARN("ERROR::FRAMEBUFFER:: Framebuffer is not complete! Status: " + std::to_string(status));
	Unbind();
	return isComplete;
}

void FrameBuffer::Blit(const FrameBuffer& target) const
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_RendererId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.GetId());
	glBlitFramebuffer(0, 0, m_Width, m_Height, 0, 0, m_Width, m_Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void FrameBuffer::MarkAsNoColorBuffer()
{
	Bind();
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	Unbind();
}

const Texture& FrameBuffer::GetColorTexture(int colorIndex) const
{
	for (const Attachment& a : m_Attachments)
		if (a.target == AttachmentTarget::Color && a.colorIndex == colorIndex)
			if (auto* tex = std::get_if<Texture>(&a.storage))
				return *tex;
	SPHWARN("Was not able to locate Color attachment for framebuffer.");
}

const Texture& FrameBuffer::GetDepthTexture() const
{
	for (const Attachment& a : m_Attachments)
		if (a.target == AttachmentTarget::Depth)
			if (auto* tex = std::get_if<Texture>(&a.storage))
				return *tex;
	SPHWARN("Was not able to locate depth attachment for framebuffer.");
}

const CubeMap& FrameBuffer::GetDepthCubeMap() const
{
	for (const Attachment& a : m_Attachments)
		if (a.target == AttachmentTarget::Depth)
			if (auto* tex = std::get_if<CubeMap>(&a.storage))
				return *tex;
	SPHWARN("Was not able to locate depth attachment for framebuffer.");
}

FrameBuffer::~FrameBuffer()
{
	if (m_RendererId != 0) glDeleteFramebuffers(1, &m_RendererId);
}

FrameBuffer::FrameBuffer(FrameBuffer && other) noexcept
{
	m_RendererId = other.m_RendererId;
	m_Width = other.m_Width;
	m_Height = other.m_Height;
	m_Attachments = std::move(other.m_Attachments);

	other.m_RendererId = 0;
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer && other) noexcept
{
	if (this == &other) return *this;

	if (m_RendererId != 0) glDeleteFramebuffers(1, &m_RendererId);

	m_RendererId = other.m_RendererId;
	m_Width = other.m_Width;
	m_Height = other.m_Height;
	m_Attachments = std::move(other.m_Attachments);

	other.m_RendererId = 0;

	return *this;
}
