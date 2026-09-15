#include "RenderLayer.h"
#include "PhysicsSolver.h"

RenderLayer::RenderLayer(std::shared_ptr<SceneData> sceneData) : m_SceneData(sceneData)
{
    Primitive circlePrimitive = Primitives::Circle(400);
    m_CircleVbo = VertexBuffer(circlePrimitive);
    m_CircleIbo = IndexBuffer(circlePrimitive);

    m_CircleVao.RecordVBOLayout(m_CircleVbo, { 3 });
    m_CircleVao.RecordIndexBuffer(m_CircleIbo);

    m_TestShader = ShaderProgram("resources/shaders/TestShader.shader");

    m_RenderFbo = FrameBuffer(m_BoundaryConfig.ViewportSize.x, m_BoundaryConfig.ViewportSize.y, 4);
    m_RenderFbo.AddAttachment(AttachmentTarget::Color, AttachmentStorage::Texture);
    m_RenderFbo.AddAttachment(AttachmentTarget::DepthStencil, AttachmentStorage::RenderBuffer);
    m_RenderFbo.Validate();

    m_ResolveFbo = FrameBuffer(m_BoundaryConfig.ViewportSize.x, m_BoundaryConfig.ViewportSize.y);
    m_ResolveFbo.AddAttachment(AttachmentTarget::Color, AttachmentStorage::Texture);
    m_ResolveFbo.Validate();
}

RenderLayer::~RenderLayer()
{}

void RenderLayer::OnUpdate(double ts)
{
    PhysicsSolver::AdvanceParticles(m_SceneData->FluidData.Particles, ts, m_BoundaryConfig);
}



void RenderLayer::OnRender()
{
    BeginFrame();

    ReadSceneData();

    for (int i = 0; i < m_SceneData->FluidData.Particles.size(); i++) {
        Particle particle = m_SceneData->FluidData.Particles[i];
        glm::mat4 model = glm::translate(glm::mat4(1), particle.Position);
        model = glm::scale(model, glm::vec3(particle.Radius));

        m_TestShader.SetUniformMat4f("u_Model", model);
        m_Renderer.DrawElements(m_CircleVao, m_TestShader);
    }


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
    m_SceneData->BoundaryData = m_BoundaryConfig;
}

void RenderLayer::ReadSceneData()
{

}

float RenderLayer::getAspectRatio()
{
    return (float)m_BoundaryConfig.ViewportSize.x / m_BoundaryConfig.ViewportSize.y;
}

void RenderLayer::OnGuiViewportSizeChanged(int width, int height)
{
    m_BoundaryConfig.ViewportSize = { width, height };
    glViewport(0, 0, m_BoundaryConfig.ViewportSize.x, m_BoundaryConfig.ViewportSize.y);
    m_RenderFbo.Resize(m_BoundaryConfig.ViewportSize.x, m_BoundaryConfig.ViewportSize.y);
    m_ResolveFbo.Resize(m_BoundaryConfig.ViewportSize.x, m_BoundaryConfig.ViewportSize.y);

    m_TestShader.SetUniformMat4f("u_Proj", glm::ortho(0.0f, m_BoundaryConfig.ViewportSize.x, 0.0f, m_BoundaryConfig.ViewportSize.y, -1.0f, 1.0f));

    OnRender();
}

