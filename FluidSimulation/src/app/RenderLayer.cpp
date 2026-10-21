#include "RenderLayer.h"
#include "Physics/PhysicsSolver.h"
#include <random>

RenderLayer::RenderLayer(std::shared_ptr<SceneData> sceneData) : m_SceneData(sceneData)
{
    Primitive circlePrimitive = Primitives::Circle(400);
    m_CircleVbo = VertexBuffer(circlePrimitive);
    m_CircleIbo = IndexBuffer(circlePrimitive);
    m_CircleVao.RecordVBOLayout(m_CircleVbo, { 3 });
    m_CircleVao.RecordIndexBuffer(m_CircleIbo);

    Primitive squarePrimitive = Primitives::NDCQuad();
    m_QuadVbo = VertexBuffer(squarePrimitive);
    m_QuadIbo = IndexBuffer(squarePrimitive);
    m_QuadVao.RecordVBOLayout(m_QuadVbo, {3, 2});
    m_QuadVao.RecordIndexBuffer(m_QuadIbo);

    m_BallShader = ShaderProgram("resources/shaders/BallShader.shader");
    m_KernelShader = ShaderProgram("resources/shaders/KernelShader.shader");

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

    const std::vector<Particle>& particles = m_SceneData->PhysicsState->Particles;

    int specimenIndex = particles.size() == 0 ? -1 : particles.size() / 2;

    if (!(specimenIndex == -1)) {
        const Particle& specimen = particles[specimenIndex];
        float h = m_SceneData->PhysicsState->FluidParameters.SmoothingRadius;
        float h2 = h * h;

        //draw smoothing radius
        glm::mat4 model = glm::translate(glm::mat4(1), specimen.Position);
        model = glm::scale(model, glm::vec3(h, h, 1));
        m_KernelShader.SetUniformMat4f("u_Model", model);
        RenderCommand::DrawElements(m_QuadVao, m_KernelShader);

        //draw balls
        for (int i = 0; i < m_SceneData->PhysicsState->Particles.size(); i++) {
            const Particle& particle = m_SceneData->PhysicsState->Particles[i];
            glm::mat4 model = glm::translate(glm::mat4(1), particle.Position);
            model = glm::scale(model, glm::vec3(PhysicsSolver::BallRadius));

            glm::vec3 d = particle.Position - specimen.Position;
            float r2 = glm::dot(d, d);

            m_BallShader.SetUniform1i("u_IsSpecimen", i == specimenIndex ? true : false);
            m_BallShader.SetUniform1i("u_WithinRadius", r2 <= h2);
            m_BallShader.SetUniformMat4f("u_Model", model);
            RenderCommand::DrawElements(m_CircleVao, m_BallShader);
        }





    }








    WriteSceneData();

    EndFrame();
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

void RenderLayer::InitialiseParticleData()
{
    int numRows = 10;
    int numCols = 10;

    std::vector<Particle> particles;
    particles.reserve(numRows * numCols);

    int stepY = m_SceneData->PhysicsState->BoundaryParameters.ViewportSize.y / numRows;
    int stepX = m_SceneData->PhysicsState->BoundaryParameters.ViewportSize.x / numCols;
    int offsetY = stepY / 2;
    int offsetX = stepX / 2;


    for (int row = 0; row < numRows; row++) {
        for (int col = 0; col < numCols; col++) {
            particles.emplace_back(glm::vec3{ offsetX + col * stepX, offsetY + row * stepY, 0 }, glm::vec3{});
        }
    }
    m_SceneData->PhysicsState->Particles = std::move(particles);

}

int RenderLayer::RandomInt(int min, int max)
{
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

void RenderLayer::RandomiseParticleData()
{
    int count = 200;

    std::vector<Particle> particles;
    particles.reserve(count);

    for (int i = 0; i < count; i++)
    {
        int xPos = RandomInt(0, m_SceneData->PhysicsState->BoundaryParameters.ViewportSize.x);
        int yPos = RandomInt(0, m_SceneData->PhysicsState->BoundaryParameters.ViewportSize.y);

        particles.emplace_back(glm::vec3{ xPos, yPos, 0 }, glm::vec3{});
    }

    m_SceneData->PhysicsState->Particles = std::move(particles);

}

void RenderLayer::OnGuiViewportSizeChanged(int width, int height)
{
    m_SceneData->PhysicsState->BoundaryParameters.ViewportSize = { width, height };
    glViewport(0, 0, width, height);
    m_RenderFbo.Resize(width, height);
    m_ResolveFbo.Resize(width, height);

    m_BallShader.SetUniformMat4f("u_Proj", glm::ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f));
    m_KernelShader.SetUniformMat4f("u_Proj", glm::ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f));
    if (!m_SceneData->PhysicsState->FluidParameters.SimulationRunning) RandomiseParticleData();

    OnRender();
}

