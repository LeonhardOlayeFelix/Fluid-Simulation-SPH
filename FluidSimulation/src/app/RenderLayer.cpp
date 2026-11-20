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
    m_BoundaryTranslationsInstanceVbo = VertexBuffer::CreateEmpty();

    m_CircleVbo = VertexBuffer(circlePrimitive);
    m_CircleIbo = IndexBuffer(circlePrimitive);
    m_CircleVao.RecordVBOLayout(m_CircleVbo, { 3 });
    m_CircleVao.RecordVBOLayout(m_TranslationsInstanceVbo, { 3 });
    m_CircleVao.RecordVBOLayout(m_VelocityInstanceVbo, { 3 });
    m_CircleVao.SetAttribDivisor(1, 1);
    m_CircleVao.SetAttribDivisor(2, 1);
    m_CircleVao.RecordIndexBuffer(m_CircleIbo);

    m_BoundaryCircleVbo = VertexBuffer(circlePrimitive);
    m_BoundaryCircleIbo = IndexBuffer(circlePrimitive);
    m_BoundaryCircleVao.RecordVBOLayout(m_BoundaryCircleVbo, { 3 });
    m_BoundaryCircleVao.RecordVBOLayout(m_BoundaryTranslationsInstanceVbo, { 3 });
    m_BoundaryCircleVao.SetAttribDivisor(1, 1);
    m_BoundaryCircleVao.RecordIndexBuffer(m_BoundaryCircleIbo);

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
    m_KernelRadiusFallOffShader = ShaderProgram("resources/shaders/2D/KernelRadiusFallOffShader.shader");
    m_KernelRadiusOutlineShader = ShaderProgram("resources/shaders/2D/KernelRadiusOutlineShader.shader");
    m_BoundingBoxOutlineShader = ShaderProgram("resources/shaders/2D/BoundingBoxOutlineShader.shader");
    m_WhiteBallShader = ShaderProgram("resources/shaders/2D/WhiteBallShader.shader");

    m_KernelBallShader = ShaderProgram("resources/shaders/2D/KernelShapeShader.shader");
    m_ArrowShader = ShaderProgram("resources/shaders/2D/ArrowShader.shader");

    m_MatricesUbo = UniformBuffer(sizeof(glm::mat4));
    m_MatricesUbo.BindToPoint(0);

    BoundingBox Viewport = m_SceneData->PhysicsState->BoundaryParameters.Viewport;

    m_RenderFbo = FrameBuffer(Viewport.width(), Viewport.height(), 6);
    m_RenderFbo.AddAttachment(AttachmentTarget::Color, AttachmentStorage::Texture);
    m_RenderFbo.AddAttachment(AttachmentTarget::DepthStencil, AttachmentStorage::RenderBuffer);

    m_ResolveFbo = FrameBuffer(Viewport.width(), Viewport.height());
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

    const std::vector<FluidParticle>& fluidParticles = m_SceneData->PhysicsState->FluidParticles;
    const std::vector<BoundaryParticle>& boundaryParticles = m_SceneData->PhysicsState->BoundaryParticles;

    if (!fluidParticles.empty())
    {
        int specimenIndex = fluidParticles.size() / 4;
        float smoothingRadius = m_SceneData->PhysicsState->FluidParameters.SmoothingRadius;

        UpdateFluidParticleInstanceBuffers(fluidParticles);

        if (m_SceneData->ShowKernel)
            DrawKernelRadiusFallOff(fluidParticles[specimenIndex].Position, smoothingRadius); //outline needs to appear behind

        DrawFluidParticles(fluidParticles, specimenIndex, smoothingRadius);

        if (m_SceneData->ShowKernel)
            DrawKernelRadiusOutline(fluidParticles[specimenIndex].Position, smoothingRadius); //outline needs to appear in front

        if (m_SceneData->ShowForceArrow)
            DrawForceArrows(fluidParticles);
    }

    if (m_SceneData->ShowParticleDomain) 
    {
        DrawBoundingBox(m_SceneData->PhysicsState->BoundaryParameters.ParticleDomain);



    }

    if (m_SceneData->ShowBoundaryParticles) 
    {
        UpdateBoundaryParticleInstanceBuffers(boundaryParticles);
        DrawBoundaryParticles(boundaryParticles);
    }

    WriteSceneData();

    EndFrame();
}

void RenderLayer::DrawKernelRadiusFallOff(glm::vec3 position, float h)
{
    glm::mat4 model = glm::scale(glm::translate(glm::mat4(1.0f), position), glm::vec3(h, h, 1.0f));
    m_KernelRadiusFallOffShader.SetUniformMat4f("u_Model", model);

    RenderCommand::DrawElements(m_QuadVao, m_KernelRadiusFallOffShader);
}

void RenderLayer::DrawKernelRadiusOutline(glm::vec3 position, float h)
{
    glm::mat4 model = glm::scale(glm::translate(glm::mat4(1.0f), position), glm::vec3(h, h, 1.0f));
    m_KernelRadiusOutlineShader.SetUniformMat4f("u_Model", model);

    RenderCommand::DrawElements(m_QuadVao, m_KernelRadiusOutlineShader);
}

void RenderLayer::DrawBoundingBox(BoundingBox box)
{
    glm::vec2 scale = box.ndcScaleFactor();
    glm::vec2 trans = box.ndcTranslation();

    glm::mat4 model = glm::scale(glm::translate(glm::mat4(1.0f), { trans.x, trans.y, 0 }), { scale.x, scale.y, 1.0f });
    m_BoundingBoxOutlineShader.SetUniformMat4f("u_Model", model);

    RenderCommand::DrawElements(m_QuadVao, m_BoundingBoxOutlineShader);

}

void RenderLayer::DrawForceArrows(const std::vector<FluidParticle>& particles)
{
    m_ArrowShader.SetUniform1f("u_ArrowLengthMax", m_SceneData->ArrowLengthMax);
    m_ArrowShader.SetUniform1f("u_ArrowWidth", m_SceneData->ArrowWidth);
    m_ArrowShader.SetUniform1f("u_MaxForceSize", getMaxForceLength(particles));
    m_ArrowShader.SetUniform1i("u_NegateDirection", m_SceneData->FlipForceArrow);
    
    RenderCommand::DrawElementsInstanced(m_ArrowVao, m_ArrowShader, particles.size());
}

void RenderLayer::DrawFluidParticles(const std::vector<FluidParticle>& particles, int specimenIndex, float smoothingRadius)
{
    const FluidParameters& fluid = m_SceneData->PhysicsState->FluidParameters;
    const float h2 = smoothingRadius * smoothingRadius;
    ShaderProgram& shader = m_VelocityParticleShader;

    shader.SetUniform1f("u_BallRadius", (m_SceneData->MatchBallSizeToSpacing ? fluid.ParticleSpacing / 2 : PhysicsSolver::FluidParticleRadius));
    shader.SetUniform1i("u_SpecimenIndex", specimenIndex);
    shader.SetUniform1f("u_MaxVelocitySize", m_SceneData->VMax);

    if (m_SceneData->ShowKernel)
    {
        shader.SetUniform1f("u_SmoothingRadiusSquared", h2);
        shader.SetUniform3f("u_SpecimenPosition", particles[specimenIndex].Position);
    }

    RenderCommand::DrawElementsInstanced(m_CircleVao, shader, particles.size());
}

void RenderLayer::DrawBoundaryParticles(const std::vector<BoundaryParticle>& boundaryParticles)
{
    m_WhiteBallShader.SetUniform1f("u_BallRadius", PhysicsSolver::BoundaryParticleRadius);

    RenderCommand::DrawElementsInstanced(m_BoundaryCircleVao, m_WhiteBallShader, boundaryParticles.size());

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

void RenderLayer::UpdateFluidParticleInstanceBuffers(const std::vector<FluidParticle>& particles)
{
    m_Translations.resize(particles.size());
    m_ForceDensities.resize(particles.size());
    m_Velocities.resize(particles.size());

    for (int i = 0; i < m_Translations.size(); i++) {
        m_Translations[i] = particles[i].Position;
        m_ForceDensities[i] = particles[i].GetTotalResultantForceDensity();
        m_Velocities[i] = particles[i].Velocity;
    }



    m_TranslationsInstanceVbo.UpdateData(m_Translations.data(), m_Translations.size() * 3 * sizeof(float));
    m_ForceDensityInstanceVbo.UpdateData(m_ForceDensities.data(), m_ForceDensities.size() * 3 * sizeof(float));
    m_VelocityInstanceVbo.UpdateData(m_Velocities.data(), m_Velocities.size() * 3 * sizeof(float));
}

void RenderLayer::UpdateBoundaryParticleInstanceBuffers(const std::vector<BoundaryParticle>& particles)
{
    m_BoundaryTranslations.resize(particles.size());

    for (int i = 0; i < m_BoundaryTranslations.size(); i++) {
        m_BoundaryTranslations[i] = particles[i].Position;
    }

    m_BoundaryTranslationsInstanceVbo.UpdateData(m_BoundaryTranslations.data(), m_BoundaryTranslations.size() * 3 * sizeof(float));
}

float RenderLayer::getAspectRatio()
{
    BoundingBox viewport = m_SceneData->PhysicsState->BoundaryParameters.Viewport;

    return viewport.width() / viewport.height();
}

float RenderLayer::getMaxForceLength(const std::vector<FluidParticle>& particles)
{
    float max = 0;

    for (const FluidParticle& particle : particles)
        max = std::max(max, glm::dot(particle.GetTotalResultantForceDensity(), particle.GetTotalResultantForceDensity()));

    return std::sqrt(max);
}

void RenderLayer::OnGuiViewportSizeChanged(int width, int height)
{
    float d = m_SceneData->PhysicsState->FluidParameters.ParticleSpacing;

    m_SceneData->PhysicsState->BoundaryParameters.Viewport = BoundingBox({0, 0}, {width, height});
    m_SceneData->PhysicsState->BoundaryParameters.ParticleDomain = BoundingBox({ 0 + 2 * d, 0 + 2 * d }, { width - 2 * d, height - 2 * d });

    glViewport(0, 0, width, height);
    m_RenderFbo.Resize(width, height);
    m_ResolveFbo.Resize(width, height);

    glm::mat4 newProjectionMatrix = glm::ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f);
    m_MatricesUbo.SetData(glm::value_ptr(newProjectionMatrix), sizeof(glm::mat4), 0);

    OnRender();
}

