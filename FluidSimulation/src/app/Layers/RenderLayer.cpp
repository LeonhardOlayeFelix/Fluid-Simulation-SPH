#include <random>

#include "RenderLayer.h"
#include "../Physics/PhysicsSolver.h"

RenderLayer::RenderLayer(std::shared_ptr<SceneData> sceneData) : m_SceneData(sceneData)
{
    Primitive circlePrimitive = Primitives::Circle(16);
    Primitive squarePrimitive = Primitives::NDCQuad();

    m_FluidForceDensityInstanceVbo = VertexBuffer::CreateEmpty();
    m_FluidTranslationsInstanceVbo = VertexBuffer::CreateEmpty();
    m_FluidVelocityInstanceVbo = VertexBuffer::CreateEmpty();
    m_FluidColorFieldInstanceVbo = VertexBuffer::CreateEmpty();
    m_BoundaryTranslationsInstanceVbo = VertexBuffer::CreateEmpty();

    m_CircleVbo = VertexBuffer(circlePrimitive);
    m_CircleIbo = IndexBuffer(circlePrimitive);
    m_CircleVao.RecordVBOLayout(m_CircleVbo, { 3 });
    m_CircleVao.RecordVBOLayout(m_FluidTranslationsInstanceVbo, { 3 });
    m_CircleVao.RecordVBOLayout(m_FluidVelocityInstanceVbo, { 3 });
    m_CircleVao.RecordVBOLayout(m_FluidColorFieldInstanceVbo, { 3 });
    m_CircleVao.SetAttribDivisor(1, 1);
    m_CircleVao.SetAttribDivisor(2, 1);
    m_CircleVao.SetAttribDivisor(3, 1);
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
    m_ArrowVao.RecordVBOLayout(m_FluidTranslationsInstanceVbo, { 3 });
    m_ArrowVao.RecordVBOLayout(m_FluidForceDensityInstanceVbo, { 3 });
    m_ArrowVao.RecordIndexBuffer(m_ArrowIbo);
    m_ArrowVao.SetAttribDivisor(2, 1);
    m_ArrowVao.SetAttribDivisor(3, 1);

    m_PlainParticleShader =     ShaderProgram("resources/shaders/2D/PlainParticleShader.shader");
    m_SpecimenParticleShader =  ShaderProgram("resources/shaders/2D/SpecimenParticleShader.shader");
    m_VelocityParticleShader =  ShaderProgram("resources/shaders/2D/VelocityParticleShader.shader");
    m_SurfaceParticleShader =   ShaderProgram("resources/shaders/2D/SurfaceTensionParticleShader.shader");
    m_BoundaryParticleShader =  ShaderProgram("resources/shaders/2D/BoundaryParticleShader.shader");

    m_KernelRadiusFallOffShader =   ShaderProgram("resources/shaders/2D/KernelRadiusFallOffShader.shader");
    m_KernelRadiusOutlineShader =   ShaderProgram("resources/shaders/2D/KernelRadiusOutlineShader.shader");
    m_BoundingBoxOutlineShader =    ShaderProgram("resources/shaders/2D/BoundingBoxOutlineShader.shader");

    m_ArrowShader = ShaderProgram("resources/shaders/2D/ArrowShader.shader");

    m_MatricesUbo = UniformBuffer(sizeof(glm::mat4));
    m_MatricesUbo.BindToPoint(0);

    BoundingBox Viewport = m_SceneData->Viewport;

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
    if (m_SceneData->ImGuiViewportSizeChanged)
        OnGuiViewportSizeChanged();
}

void RenderLayer::OnRender()
{
    BeginFrame();

    ReadSceneData();

    const std::vector<FluidParticle>& fluidParticles = m_SceneData->PhysicsData->FluidParticles;
    const std::vector<BoundaryParticle>& boundaryParticles = m_SceneData->PhysicsData->BoundaryParticles;

    if (!fluidParticles.empty())
    {
        UpdateFluidParticleInstanceBuffers(fluidParticles);

        if (m_SceneData->ShowKernelRadius && m_SceneData->FluidParticleShader == FluidParticleShaderTypes::SpecimenShader)
            DrawKernelRadiusFallOff(fluidParticles[m_SceneData->SpecimenIndex].Position);

        DrawFluidParticles(fluidParticles);

        if (m_SceneData->ShowKernelRadius && m_SceneData->FluidParticleShader == FluidParticleShaderTypes::SpecimenShader)
            DrawKernelRadiusOutline(fluidParticles[m_SceneData->SpecimenIndex].Position);

        if (m_SceneData->ShowForceArrow)
            DrawForceArrows(fluidParticles);
    }

    if (m_SceneData->ShowParticleDomain) 
    {
        DrawBoundingBox(m_SceneData->PhysicsData->BoundaryParameters.ParticleDomain);
    }

    if (m_SceneData->ShowBoundaryParticles) 
    {
        UpdateBoundaryParticleInstanceBuffers(boundaryParticles);
        DrawBoundaryParticles(boundaryParticles);
    }

    WriteSceneData();

    EndFrame();
}

void RenderLayer::DrawKernelRadiusFallOff(glm::vec3 position)
{
    float h = m_SceneData->PhysicsData->FluidParameters.SmoothingRadius;

    glm::mat4 model = glm::scale(glm::translate(glm::mat4(1.0f), position), glm::vec3(h, h, 1.0f));
    m_KernelRadiusFallOffShader.SetUniformMat4f("u_Model", model);

    RenderCommand::DrawElements(m_QuadVao, m_KernelRadiusFallOffShader);
}

void RenderLayer::DrawKernelRadiusOutline(glm::vec3 position)
{
    float h = m_SceneData->PhysicsData->FluidParameters.SmoothingRadius;

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

void RenderLayer::DrawFluidParticles(const std::vector<FluidParticle>& particles)
{
    ShaderProgram& shader = selectFluidParticleShader();

    setFluidParticleShaderUniforms(shader);

    RenderCommand::DrawElementsInstanced(m_CircleVao, shader, particles.size());
}

void RenderLayer::DrawBoundaryParticles(const std::vector<BoundaryParticle>& boundaryParticles)
{
    m_BoundaryParticleShader.SetUniform1f("u_BallRadius", PhysicsSolver::BoundaryParticleRadius);

    RenderCommand::DrawElementsInstanced(m_BoundaryCircleVao, m_BoundaryParticleShader, boundaryParticles.size());

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
    m_FluidTranslations.resize(particles.size());
    m_FluidForceDensities.resize(particles.size());
    m_FluidVelocities.resize(particles.size());
    m_FluidColorFieldGradients.resize(particles.size());

    for (int i = 0; i < m_FluidTranslations.size(); i++) {
        m_FluidTranslations[i] = particles[i].Position;
        m_FluidForceDensities[i] = particles[i].ResultantForceDensity;
        m_FluidVelocities[i] = particles[i].Velocity;
        m_FluidColorFieldGradients[i] = particles[i].ColorFieldGradient;
    }

    m_FluidTranslationsInstanceVbo.UpdateData(m_FluidTranslations.data(), m_FluidTranslations.size() * 3 * sizeof(float));
    m_FluidForceDensityInstanceVbo.UpdateData(m_FluidForceDensities.data(), m_FluidForceDensities.size() * 3 * sizeof(float));
    m_FluidVelocityInstanceVbo.UpdateData(m_FluidVelocities.data(), m_FluidVelocities.size() * 3 * sizeof(float));
    m_FluidColorFieldInstanceVbo.UpdateData(m_FluidColorFieldGradients.data(), m_FluidColorFieldGradients.size() * 3 * sizeof(float));
}

void RenderLayer::UpdateBoundaryParticleInstanceBuffers(const std::vector<BoundaryParticle>& particles)
{
    m_BoundaryTranslations.resize(particles.size());

    for (int i = 0; i < m_BoundaryTranslations.size(); i++) {
        m_BoundaryTranslations[i] = particles[i].Position;
    }

    m_BoundaryTranslationsInstanceVbo.UpdateData(m_BoundaryTranslations.data(), m_BoundaryTranslations.size() * 3 * sizeof(float));
}

ShaderProgram& RenderLayer::selectFluidParticleShader()
{

    if (m_SceneData->FluidParticleShader == FluidParticleShaderTypes::PlainShader)
        return m_PlainParticleShader;
    if (m_SceneData->FluidParticleShader == FluidParticleShaderTypes::SpecimenShader)
        return m_SpecimenParticleShader;
    if (m_SceneData->FluidParticleShader == FluidParticleShaderTypes::VelocityShader)
        return m_VelocityParticleShader;
    if (m_SceneData->FluidParticleShader == FluidParticleShaderTypes::SurfaceTensionShader)
        return m_SurfaceParticleShader;

    return m_PlainParticleShader;
}

void RenderLayer::setFluidParticleShaderUniforms(ShaderProgram& shader)
{
    const FluidParameters& fluid = m_SceneData->PhysicsData->FluidParameters;
    const std::vector<FluidParticle>& fluidParticles = m_SceneData->PhysicsData->FluidParticles;
    const float ballRadius = m_SceneData->MatchBallSizeToSpacing ? fluid.ParticleSpacing / 2 : PhysicsSolver::FluidParticleRadius;
    size_t specimenIndex = std::min((size_t)m_SceneData->SpecimenIndex, fluidParticles.size() - 1);

    if (m_SceneData->FluidParticleShader == FluidParticleShaderTypes::PlainShader)
    {
        shader.SetUniform3f("u_ParticleColor", m_SceneData->PlainParticleColor);
        shader.SetUniform1f("u_BallRadius", (m_SceneData->MatchBallSizeToSpacing ? fluid.ParticleSpacing / 2 : PhysicsSolver::FluidParticleRadius));
    }
    if (m_SceneData->FluidParticleShader == FluidParticleShaderTypes::SpecimenShader)
    {
        shader.SetUniform1i("u_SpecimenIndex", specimenIndex);
        shader.SetUniform1i("u_HighlightNeighbors", m_SceneData->HighlightSpecimenNeighbors);

        shader.SetUniform3f("u_ParticleColor", m_SceneData->PlainParticleColor);
        shader.SetUniform3f("u_SpecimenColor", m_SceneData->SpecimenParticleColor);
        shader.SetUniform3f("u_NeighborColor", m_SceneData->SpecimenNeighborColor);
        shader.SetUniform3f("u_SpecimenPosition", fluidParticles[specimenIndex].Position);

        shader.SetUniform1f("u_SmoothingRadiusSquared", fluid.SmoothingRadius * fluid.SmoothingRadius);
        shader.SetUniform1f("u_SpecimenBallRadius", fluid.ParticleSpacing / 2);
    }
    if (m_SceneData->FluidParticleShader == FluidParticleShaderTypes::VelocityShader)
    {
        shader.SetUniform1f("u_MaxVelocitySize", m_SceneData->VMax);
    }
    if (m_SceneData->FluidParticleShader == FluidParticleShaderTypes::SurfaceTensionShader)
    {
        shader.SetUniform3f("u_ParticleColor", m_SceneData->PlainParticleColor);
        shader.SetUniform3f("u_SurfaceParticleColor", m_SceneData->SurfaceParticleColor);
        shader.SetUniform1f("u_SurfaceTensionThreshold", fluid.SurfaceTensionThreshold);
    }

    shader.SetUniform1f("u_BallRadius", ballRadius);
}

float RenderLayer::getAspectRatio()
{
    BoundingBox viewport = m_SceneData->Viewport;

    return viewport.width() / viewport.height();
}

float RenderLayer::getMaxForceLength(const std::vector<FluidParticle>& particles)
{
    float max = 0;

    for (const FluidParticle& particle : particles)
        max = std::max(max, glm::dot(particle.ResultantForceDensity, particle.ResultantForceDensity));

    return std::sqrt(max);
}

void RenderLayer::OnGuiViewportSizeChanged()
{
    int width = m_SceneData->ContentRegionAvailValue.x;
    int height = m_SceneData->ContentRegionAvailValue.y;

    PhysicsData& state = *m_SceneData->PhysicsData;

    float d = state.FluidParameters.ParticleSpacing;

    BoundingBox& viewportDomain = m_SceneData->Viewport;
    BoundingBox& particleDomain = state.BoundaryParameters.ParticleDomain;

    viewportDomain = BoundingBox({0, 0}, {width, height});
    particleDomain = viewportDomain.getInset(2 * d);


    glViewport(0, 0, width, height);
    m_RenderFbo.Resize(width, height);
    m_ResolveFbo.Resize(width, height);

    glm::mat4 newProjectionMatrix = glm::ortho(viewportDomain.xmin(), viewportDomain.xmax(), viewportDomain.ymin(), viewportDomain.ymax(), -1.0f, 1.0f);
    m_MatricesUbo.SetData(glm::value_ptr(newProjectionMatrix), sizeof(glm::mat4), 0);

    if (!state.FluidParameters.SimulationRunning && m_SceneData->InGridFormation)
        m_SceneData->PhysicsData->OrganiseFluidParticlesIntoGrid(state.FluidParameters.FluidParticleCount, d);

    m_SceneData->PhysicsData->OrganiseBoundaryParticles();

    OnRender();
    m_SceneData->ImGuiViewportSizeChanged = false;
}

