#include "RenderLayer.h"
#include "Physics/PhysicsSolver.h"
#include <random>

RenderLayer::RenderLayer(std::shared_ptr<SceneData> sceneData) : m_SceneData(sceneData)
{
    Primitive circlePrimitive = Primitives::Circle(16);
    Primitive squarePrimitive = Primitives::NDCQuad();

    m_DensityGradientsInstanceVbo = VertexBuffer::CreateEmpty();
    m_TranslationsInstanceVbo = VertexBuffer::CreateEmpty();

    m_CircleVbo = VertexBuffer(circlePrimitive);
    m_CircleIbo = IndexBuffer(circlePrimitive);
    m_CircleVao.RecordVBOLayout(m_CircleVbo, { 3 });
    m_CircleVao.RecordVBOLayout(m_TranslationsInstanceVbo, { 3 });
    m_CircleVao.SetAttribDivisor(1, 1);
    m_CircleVao.RecordIndexBuffer(m_CircleIbo);

    m_QuadVbo = VertexBuffer(squarePrimitive);
    m_QuadIbo = IndexBuffer(squarePrimitive);
    m_QuadVao.RecordVBOLayout(m_QuadVbo, {3, 2});
    m_QuadVao.RecordIndexBuffer(m_QuadIbo);

    m_ArrowVbo = VertexBuffer(squarePrimitive);
    m_ArrowIbo = IndexBuffer(squarePrimitive);
    m_ArrowVao.RecordVBOLayout(m_ArrowVbo, { 3, 2 });
    m_ArrowVao.RecordVBOLayout(m_TranslationsInstanceVbo, { 3 });
    m_ArrowVao.RecordVBOLayout(m_DensityGradientsInstanceVbo, { 3 });
    m_ArrowVao.RecordIndexBuffer(m_ArrowIbo);
    m_ArrowVao.SetAttribDivisor(2, 1);
    m_ArrowVao.SetAttribDivisor(3, 1);

    m_DefaultBallShader = ShaderProgram("resources/shaders/2D/DefaultBallShader.shader");
    m_KernelBallShader = ShaderProgram("resources/shaders/2D/WithinKernelBallShader.shader");
    m_KernelRadiusShader = ShaderProgram("resources/shaders/2D/KernelShapeShader.shader");
    m_ArrowShader = ShaderProgram("resources/shaders/2D/ArrowShader.shader");

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
        float h = m_SceneData->PhysicsState->FluidParameters.SmoothingRadius;

        int specimenIndex = particles.size() == 0 ? -1 : particles.size() / 2;

        Particle& specimen = particles[specimenIndex];

        UpdateInstanceBuffers(particles);

        if (m_SceneData->showKernel)
            DrawKernelRadius(specimen.Position, h);


        if (m_SceneData->showDensityGradient)
        {
            float max = 0;

            for (const Particle& particle : particles)
                max = std::max(max, glm::dot(particle.DensityGradient, particle.DensityGradient));

            DrawDensityGradients(std::sqrt(max), particles.size());
        }

        DrawParticles(specimen, specimenIndex, h);
    }

    

    WriteSceneData();

    EndFrame();
}

void RenderLayer::DrawKernelRadius(glm::vec3 position, float h)
{
    m_KernelRadiusShader.SetUniformMat4f("u_Model", glm::scale(glm::translate(glm::mat4(1.0f), position), glm::vec3(h, h, 1.0f)));

    RenderCommand::DrawElements(m_QuadVao, m_KernelRadiusShader);
}

void RenderLayer::DrawDensityGradients(float MaxDensityGradientSize, int particleCount)
{
    m_ArrowShader.SetUniform1f("u_ArrowLengthMax", m_SceneData->ArrowLengthMax);
    m_ArrowShader.SetUniform1f("u_ArrowWidth", m_SceneData->ArrowWidth);
    m_ArrowShader.SetUniform1f("u_MaxDensityGradientSize", MaxDensityGradientSize);
    m_ArrowShader.SetUniform1i("u_NegateGradient", m_SceneData->flipDensityGradient);
    
    RenderCommand::DrawElementsInstanced(m_ArrowVao, m_ArrowShader, particleCount);
}

void RenderLayer::DrawParticles(const Particle& specimen, int specimenIndex, float h)
{
    const float h2 = h * h;
    const bool showKernel = m_SceneData->showKernel;
    ShaderProgram& shader = showKernel ? m_KernelBallShader : m_DefaultBallShader;
    const std::vector<Particle>& particles = m_SceneData->PhysicsState->Particles;
    
    shader.SetUniform1f("u_BallRadius", PhysicsSolver::BallRadius);
    shader.SetUniform1i("u_SpecimenIndex", specimenIndex);

    if (showKernel) {
        shader.SetUniform1f("u_SmoothingRadiusSquared", h2);
        shader.SetUniform3f("u_SpecimenPosition", specimen.Position);
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
    std::vector<glm::vec3> translations(particles.size());
    std::vector<glm::vec3> densityGradients(particles.size());

    for (int i = 0; i < translations.size(); i++) {
        translations[i] = particles[i].Position;
        densityGradients[i] = particles[i].DensityGradient;
    }

    m_TranslationsInstanceVbo.UpdateData(translations.data(), translations.size() * 3 * sizeof(float));
    m_DensityGradientsInstanceVbo.UpdateData(densityGradients.data(), densityGradients.size() * 3 * sizeof(float));
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

    OnRender();
}

