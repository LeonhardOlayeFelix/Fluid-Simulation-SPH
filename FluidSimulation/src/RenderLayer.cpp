#include "RenderLayer.h"

RenderLayer::RenderLayer(std::shared_ptr<SceneData> sceneData) : m_SceneData(sceneData)
{
    Primitive circlePrimitive = Primitives::Circle(400);
    m_CircleVbo = VertexBuffer(circlePrimitive);
    m_CircleIbo = IndexBuffer(circlePrimitive);

    VertexBufferLayout circleVbl;
    circleVbl.push<float>(3);
    m_CircleVao.RecordVBOLayout(m_CircleVbo, circleVbl);
    m_CircleVao.RecordIndexBuffer(m_CircleIbo);

    Primitive quadPrimitive = Primitives::NDCQuad();
    m_QuadVbo = VertexBuffer(quadPrimitive);
    m_QuadIbo = IndexBuffer(quadPrimitive);
    VertexBufferLayout quadVbl;
    quadVbl.push<float>(3);
    quadVbl.push<float>(2);
    m_QuadVao.RecordVBOLayout(m_QuadVbo, quadVbl);
    m_QuadVao.RecordIndexBuffer(m_QuadIbo);

    m_TestShader = ShaderProgram("resources/shaders/TestShader.shader");

    m_RenderFbo = FrameBuffer(m_ViewportSize.x, m_ViewportSize.y, 4);
    m_RenderFbo.AddAttachment(AttachmentTarget::Color, AttachmentStorage::Texture);
    m_RenderFbo.AddAttachment(AttachmentTarget::DepthStencil, AttachmentStorage::RenderBuffer);
    m_RenderFbo.Validate();

    m_ResolveFbo = FrameBuffer(m_ViewportSize.x, m_ViewportSize.y);
    m_ResolveFbo.AddAttachment(AttachmentTarget::Color, AttachmentStorage::Texture);
    m_ResolveFbo.Validate();
}

RenderLayer::~RenderLayer()
{}

void RenderLayer::OnUpdate(double ts)
{}



void RenderLayer::OnRender()
{
    BeginFrame();

    ReadSceneData();

    m_Renderer.DrawElements(m_CircleVao, m_TestShader);

    WriteSceneData();

    EndFrame();
}

void RenderLayer::BeginFrame()
{
    m_RenderFbo.Bind();
    m_Renderer.Clear();
}

void RenderLayer::EndFrame()
{
    m_RenderFbo.Blit(m_ResolveFbo);
    m_RenderFbo.Unbind();
}

void RenderLayer::WriteSceneData()
{
    m_SceneData->FramebufferTextureId = m_ResolveFbo.GetColorTexture().GetId();
    m_SceneData->ViewportSize = m_ViewportSize;
}

void RenderLayer::ReadSceneData()
{
    m_TestShader.SetUniformMat4f("u_Model", glm::scale(glm::mat4(1), glm::vec3(m_SceneData->Scale)));
}

float RenderLayer::getAspectRatio()
{
    return (float)m_ViewportSize.x / m_ViewportSize.y;
}

void RenderLayer::OnGuiViewportSizeChanged(int width, int height)
{
	m_ViewportSize = { width, height };
    glViewport(0, 0, m_ViewportSize.x, m_ViewportSize.y);
    m_RenderFbo.Resize(m_ViewportSize.x, m_ViewportSize.y);
    m_ResolveFbo.Resize(m_ViewportSize.x, m_ViewportSize.y);
    m_TestShader.SetUniformMat4f("u_Proj", glm::ortho(-getAspectRatio(), getAspectRatio(), -1.0f, 1.0f, -1.0f, 1.0f));

    OnRender();
}

