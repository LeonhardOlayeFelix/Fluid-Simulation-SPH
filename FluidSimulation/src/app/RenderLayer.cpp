#include "RenderLayer.h"
#include "Physics/PhysicsSolver.h"
#include <random>

RenderLayer::RenderLayer(std::shared_ptr<SceneData> sceneData) : m_SceneData(sceneData)
{
    Primitive circlePrimitive = Primitives::Circle(16);
    Primitive squarePrimitive = Primitives::NDCQuad();

    m_ForceDensityInstanceVbo = VertexBuffer::CreateEmpty();
    m_TranslationsInstanceVbo = VertexBuffer::CreateEmpty();
    m_VelocityInstanceVbo = VertexBuffer::CreateEmpty();

    m_CircleVbo = VertexBuffer(circlePrimitive);
    m_CircleIbo = IndexBuffer(circlePrimitive);
    m_CircleVao.RecordVBOLayout(m_CircleVbo, { 3 });
    m_CircleVao.RecordVBOLayout(m_TranslationsInstanceVbo, { 3 });
    m_CircleVao.RecordVBOLayout(m_VelocityInstanceVbo, { 3 });
    m_CircleVao.SetAttribDivisor(1, 1);
    m_CircleVao.SetAttribDivisor(2, 1);
    m_CircleVao.RecordIndexBuffer(m_CircleIbo);

    m_QuadVbo = VertexBuffer(squarePrimitive);
    m_QuadIbo = IndexBuffer(squarePrimitive);
    m_QuadVao.RecordVBOLayout(m_QuadVbo, {3, 2});
    m_QuadVao.RecordIndexBuffer(m_QuadIbo);

    m_ArrowVbo = VertexBuffer(squarePrimitive);
    m_ArrowIbo = IndexBuffer(squarePrimitive);
    m_ArrowVao.RecordVBOLayout(m_ArrowVbo, { 3, 2 });
    m_ArrowVao.RecordVBOLayout(m_TranslationsInstanceVbo, { 3 });
    m_ArrowVao.RecordVBOLayout(m_ForceDensityInstanceVbo, { 3 });
    m_ArrowVao.RecordIndexBuffer(m_ArrowIbo);
    m_ArrowVao.SetAttribDivisor(2, 1);
    m_ArrowVao.SetAttribDivisor(3, 1);

    m_ParticleShader = ShaderProgram("resources/shaders/2D/ParticleShader.shader");
    m_RadiusParticleShader = ShaderProgram("resources/shaders/2D/RadiusParticleShader.shader");
    m_VelocityParticleShader = ShaderProgram("resources/shaders/2D/VelocityParticleShader.shader");

    m_KernelBallShader = ShaderProgram("resources/shaders/2D/KernelShapeShader.shader");
    m_ArrowShader = ShaderProgram("resources/shaders/2D/ArrowShader.shader");

    m_MatricesUbo = UniformBuffer(sizeof(glm::mat4));
    m_MatricesUbo.BindToPoint(0);

    m_RenderFbo = FrameBuffer(m_SceneData->PhysicsState->BoundaryParameters.ViewportSize.x, m_SceneData->PhysicsState->BoundaryParameters.ViewportSize.y, 6);
    m_RenderFbo.AddAttachment(AttachmentTarget::Color, AttachmentStorage::Texture);
    m_RenderFbo.AddAttachment(AttachmentTarget::DepthStencil, AttachmentStorage::RenderBuffer);

    m_ResolveFbo = FrameBuffer(m_SceneData->PhysicsState->BoundaryParameters.ViewportSize.x, m_SceneData->PhysicsState->BoundaryParameters.ViewportSize.y);
    m_ResolveFbo.AddAttachment(AttachmentTarget::Color, AttachmentStorage::Texture);
}

RenderLayer::~RenderLayer()
{

}

void RenderLayer::OnUpdate(double ts)
{
    
}

void RenderLayer::OnRender()
{
    BeginFrame();

    ReadSceneData();

    std::vector<Particle>& particles = m_SceneData->PhysicsState->Particles;

    if (!particles.empty())
    {
        int specimenIndex = particles.size() / 4;
        float smoothingRadius = m_SceneData->PhysicsState->FluidParameters.SmoothingRadius;

        UpdateInstanceBuffers(particles);

        if (m_SceneData->ShowKernel)
            DrawKernelRadius(particles[specimenIndex].Position, smoothingRadius);

        if (m_SceneData->ShowForceArrow)
            DrawForceArrows(particles);

        DrawParticles(particles, specimenIndex, smoothingRadius);
    }

    WriteSceneData();

    EndFrame();
}

void RenderLayer::DrawKernelRadius(glm::vec3 position, float h)
{
    m_RadiusParticleShader.SetUniformMat4f("u_Model", glm::scale(glm::translate(glm::mat4(1.0f), position), glm::vec3(h, h, 1.0f)));

    RenderCommand::DrawElements(m_QuadVao, m_RadiusParticleShader);
}

void RenderLayer::DrawForceArrows(const std::vector<Particle>& particles)
{
    m_ArrowShader.SetUniform1f("u_ArrowLengthMax", m_SceneData->ArrowLengthMax);
    m_ArrowShader.SetUniform1f("u_ArrowWidth", m_SceneData->ArrowWidth);
    m_ArrowShader.SetUniform1f("u_MaxForceSize", getMaxForceLength(particles));
    m_ArrowShader.SetUniform1i("u_NegateDirection", m_SceneData->FlipForceArrow);
    
    RenderCommand::DrawElementsInstanced(m_ArrowVao, m_ArrowShader, particles.size());
}

void RenderLayer::DrawParticles(const std::vector<Particle>& particles, int specimenIndex, float smoothingRadius)
{
    const float h2 = smoothingRadius * smoothingRadius;
    //ShaderProgram& shader = m_ParticleShader;
    ShaderProgram& shader = m_VelocityParticleShader;

    shader.SetUniform1f("u_BallRadius", PhysicsSolver::BallRadius);
    shader.SetUniform1i("u_SpecimenIndex", specimenIndex);
    shader.SetUniform1f("u_MaxVelocitySize", m_SceneData->VMax);

    if (m_SceneData->ShowKernel)
    {
        shader.SetUniform1f("u_SmoothingRadiusSquared", h2);
        shader.SetUniform3f("u_SpecimenPosition", particles[specimenIndex].Position);
    }

    RenderCommand::DrawElementsInstanced(m_CircleVao, shader, particles.size());
}


void RenderLayer::BeginFrame()
{
    m_RenderFbo.Bind();
    RenderCommand::Clear();
}

void RenderLayer::EndFrame()
{
    m_RenderFbo.Blit(m_ResolveFbo);
    m_RenderFbo.Unbind();
}

void RenderLayer::WriteSceneData()
{
    m_SceneData->FramebufferTextureId = m_ResolveFbo.GetColorTexture().GetId();
}

void RenderLayer::ReadSceneData()
{

}

void RenderLayer::UpdateInstanceBuffers(const std::vector<Particle>& particles)
{
    m_Translations.resize(particles.size());
    m_ForceDensities.resize(particles.size());
    m_Velocities.resize(particles.size());

    for (int i = 0; i < m_Translations.size(); i++) {
        m_Translations[i] = particles[i].Position;
        m_ForceDensities[i] = particles[i].ResultantForceDensity;
        m_Velocities[i] = particles[i].Velocity;
    }

    m_TranslationsInstanceVbo.UpdateData(m_Translations.data(), m_Translations.size() * 3 * sizeof(float));
    m_ForceDensityInstanceVbo.UpdateData(m_ForceDensities.data(), m_ForceDensities.size() * 3 * sizeof(float));
    m_VelocityInstanceVbo.UpdateData(m_Velocities.data(), m_Velocities.size() * 3 * sizeof(float));
}

float RenderLayer::getAspectRatio()
{
    return (float)m_SceneData->PhysicsState->BoundaryParameters.ViewportSize.x / m_SceneData->PhysicsState->BoundaryParameters.ViewportSize.y;
}

float RenderLayer::getMaxForceLength(const std::vector<Particle>& particles)
{
    float max = 0;

    for (const Particle& particle : particles)
        max = std::max(max, glm::dot(particle.ResultantForceDensity, particle.ResultantForceDensity));

    return std::sqrt(max);
}

void RenderLayer::OnGuiViewportSizeChanged(int width, int height)
{
    m_SceneData->PhysicsState->BoundaryParameters.ViewportSize = { width, height };

    glViewport(0, 0, width, height);
    m_RenderFbo.Resize(width, height);
    m_ResolveFbo.Resize(width, height);

    glm::mat4 newProjectionMatrix = glm::ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f);
    m_MatricesUbo.SetData(glm::value_ptr(newProjectionMatrix), sizeof(glm::mat4), 0);

    OnRender();
}

