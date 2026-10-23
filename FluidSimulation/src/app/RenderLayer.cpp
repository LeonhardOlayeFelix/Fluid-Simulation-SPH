#include "RenderLayer.h"
#include "Physics/PhysicsSolver.h"
#include <random>

RenderLayer::RenderLayer(std::shared_ptr<SceneData> sceneData) : m_SceneData(sceneData)
{
    Primitive circlePrimitive = Primitives::Circle(400);
    m_CircleVbo = VertexBuffer(circlePrimitive);
    m_CircleIbo = IndexBuffer(circlePrimitive);
    m_TranslationInstanceVbo = VertexBuffer::CreateEmpty();
    m_CircleVao.RecordVBOLayout(m_CircleVbo, { 3 });
    m_CircleVao.RecordVBOLayout(m_TranslationInstanceVbo, { 3 });
    m_CircleVao.SetAttribDivisor(1, 1);
    m_CircleVao.RecordIndexBuffer(m_CircleIbo);


    Primitive squarePrimitive = Primitives::NDCQuad();
    m_QuadVbo = VertexBuffer(squarePrimitive);
    m_QuadIbo = IndexBuffer(squarePrimitive);
    m_QuadVao.RecordVBOLayout(m_QuadVbo, {3, 2});
    m_QuadVao.RecordIndexBuffer(m_QuadIbo);

    m_DefaultBallShader = ShaderProgram("resources/shaders/DefaultBallShader.shader");
    m_KernelBallShader = ShaderProgram("resources/shaders/KernelBallShader.shader");
    m_KernelRadiusShader = ShaderProgram("resources/shaders/KernelSmoothingRadiusShader.shader");
    m_ArrowShader = ShaderProgram("resources/shaders/ArrowShader.shader");

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
    float h = m_SceneData->PhysicsState->FluidParameters.SmoothingRadius;

    int specimenIndex = particles.size() == 0 ? -1 : particles.size() / 2;

    if (specimenIndex == -1) return;

    Particle& specimen = particles[specimenIndex];

    float h2 = h * h;

    if (m_SceneData->showKernel)
        DrawKernelRadius(specimen, h);


    if (m_SceneData->showDensityGradient) 
    {
        float MaxDensityGradientLength = 0;

        for (const Particle& particle : particles) {
            MaxDensityGradientLength = std::max(MaxDensityGradientLength, glm::length(particle.DensityGradient));
        }

        for (const Particle& particle : particles) {
            DrawDensityGradient(particle, MaxDensityGradientLength);
        }

    }


    DrawParticles(specimen, specimenIndex, h);

    WriteSceneData();

    EndFrame();
}

void RenderLayer::DrawKernelRadius(const Particle& specimen, float h)
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), specimen.Position);
    model = glm::scale(model, glm::vec3(h, h, 1.0f));

    m_KernelRadiusShader.SetUniformMat4f("u_Model", model);
    RenderCommand::DrawElements(m_QuadVao, m_KernelRadiusShader);
}

void RenderLayer::DrawDensityGradient(const Particle& specimen, float MaxDensityGradientLength)
{
    const glm::vec3& g = specimen.DensityGradient;
    if (glm::dot(g, g) < 1e-12f) return;

    float length = m_SceneData->ArrowLengthMax * glm::length(specimen.DensityGradient) / MaxDensityGradientLength;
    float width = (float)m_SceneData->ArrowWidth;

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(specimen.Position.x, specimen.Position.y, 0.0f));
    float angle = std::atan2(g.y, g.x) + (m_SceneData->flipDensityGradient ? glm::pi<float>() : 0.0f);
    model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(length / 2.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(length / 2.0f, width / 2.0f, 1.0f));

    m_ArrowShader.SetUniformMat4f("u_Model", model);
    RenderCommand::DrawElements(m_QuadVao, m_ArrowShader);
}

void RenderLayer::DrawParticles(const Particle& specimen, int specimenIndex, float h)
{
    const float h2 = h * h;
    const bool showKernel = m_SceneData->showKernel;
    ShaderProgram& shader = showKernel ? m_KernelBallShader : m_DefaultBallShader;
    const std::vector<Particle>& particles = m_SceneData->PhysicsState->Particles;

    std::vector<glm::vec3> translations(particles.size());

    for (int i = 0; i < translations.size(); i++)
        translations[i] = particles[i].Position;
    
    shader.SetUniformMat4f("u_Model", glm::scale(glm::mat4(1), glm::vec3(PhysicsSolver::BallRadius)));
    shader.SetUniform1i("u_SpecimenIndex", specimenIndex);

    if (showKernel) {
        shader.SetUniform1f("u_SmoothingRadiusSquared", h2);
        shader.SetUniform3f("u_SpecimenPosition", specimen.Position);
    }

    m_TranslationInstanceVbo.UpdateData(translations.data(), translations.size() * 3 * sizeof(float));
    


    RenderCommand::DrawElementsInstanced(m_CircleVao, shader, particles.size());


    /*for (int i = 0; i < particles.size(); i++) {
        const Particle& particle = particles[i];

        glm::mat4 model = glm::translate(glm::mat4(1.0f), particle.Position);
        model = glm::scale(model, glm::vec3(PhysicsSolver::BallRadius));

        shader.SetUniform1i("u_IsSpecimen", i == specimenIndex);
        shader.SetUniformMat4f("u_Model", model);

        if (showKernel) {
            glm::vec3 d = particle.Position - specimen.Position;
            shader.SetUniform1i("u_WithinRadius", glm::dot(d, d) <= h2);
        }

        RenderCommand::DrawElements(m_CircleVao, shader);
    }*/
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

float RenderLayer::getAspectRatio()
{
    return (float)m_SceneData->PhysicsState->BoundaryParameters.ViewportSize.x / m_SceneData->PhysicsState->BoundaryParameters.ViewportSize.y;
}

void RenderLayer::OnGuiViewportSizeChanged(int width, int height)
{
    m_SceneData->PhysicsState->BoundaryParameters.ViewportSize = { width, height };
    glViewport(0, 0, width, height);
    m_RenderFbo.Resize(width, height);
    m_ResolveFbo.Resize(width, height);

    m_DefaultBallShader.SetUniformMat4f("u_Proj", glm::ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f));
    m_KernelBallShader.SetUniformMat4f("u_Proj", glm::ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f));
    m_KernelRadiusShader.SetUniformMat4f("u_Proj", glm::ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f));
    m_ArrowShader.SetUniformMat4f("u_Proj", glm::ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f));
    if (!m_SceneData->PhysicsState->FluidParameters.SimulationRunning) m_SceneData->PhysicsState->RandomiseParticleData(m_SceneData->count);

    OnRender();
}

