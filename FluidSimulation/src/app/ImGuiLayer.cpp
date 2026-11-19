#include "ImGuiLayer.h"
#include "Application.h"
#include "Physics/PhysicsSolver.h"
#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_impl_glfw.h"
#include "../vendor/imgui/imgui_impl_opengl3.h"

ImGuiLayer::ImGuiLayer(std::shared_ptr<SceneData> sceneData, std::shared_ptr<RenderLayer> renderLayer) : m_SceneData(sceneData), m_RenderLayer(renderLayer)
{}

ImGuiLayer::~ImGuiLayer()
{
	Destroy();
}

void ImGuiLayer::OnUpdate(double ts)
{

}

void ImGuiLayer::OnRender()
{
	BeginFrame();

	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

	DrawViewport();
	DrawControlsPanel();
	DrawInspector();
	DrawStats();


	EndFrame();
}

void ImGuiLayer::DrawViewport()
{
    ImGui::Begin("Viewport");

    BoundingBox oldParticleDomain = m_SceneData->PhysicsState->BoundaryParameters.ParticleDomain;

    auto oldSize = oldParticleDomain.size();
    ImVec2 currSize = ImGui::GetContentRegionAvail();
    if (currSize.x != oldSize.x || currSize.y != oldSize.y) {
        m_RenderLayer->OnGuiViewportSizeChanged(currSize.x, currSize.y);
        m_SceneData->PhysicsState->RecalculateBoundaryParticles();
    }

    ImGui::Image(reinterpret_cast<void*>(m_SceneData->FramebufferTextureId), currSize, ImVec2(0, 1), ImVec2(1, 0));

    ImGui::End();
}

void ImGuiLayer::DrawControlsPanel()
{
    PhysicsState& state = *m_SceneData->PhysicsState;
    FluidParameters& fluid = state.FluidParameters;

    ImGui::Begin("Controls");

    if (ImGui::CollapsingHeader("Scene Setup", ImGuiTreeNodeFlags_DefaultOpen)) {

        bool a = ImGui::DragInt("Count", &m_SceneData->PhysicsState->FluidParameters.FluidParticleCount, 1.0, 1, 10000);
        bool c = ImGui::DragFloat("Fluid Particle Radius", &PhysicsSolver::FluidParticleRadius, 0.1f, 0.1f, 50.0f);

        if (m_SceneData->ShowBoundaryParticles) 
        {
            ImGui::DragFloat("Boundary Particle Radius", &PhysicsSolver::BoundaryParticleRadius, 0.1f, 0.1f, 50.0f);
        }

        if (a && !fluid.SimulationRunning)
        {
            m_SceneData->PhysicsState->GridParticleData(fluid.FluidParticleCount, fluid.ParticleSpacing);
        }

        if (ImGui::Button("Set"))
            m_SceneData->PhysicsState->GridParticleData(fluid.FluidParticleCount, fluid.ParticleSpacing);
    }

    if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen)) {

        if (ImGui::Button(fluid.SimulationRunning ? "Pause Simulation" : "Begin Simulation"))
            fluid.SimulationRunning = !fluid.SimulationRunning;

        ImGui::DragFloat("Collision Damping", &PhysicsSolver::CollisionDampingFactor, 0.01, 0.0f, 1.0f, "%.2f");
    }

    if (ImGui::CollapsingHeader("Fluid (SPH)", ImGuiTreeNodeFlags_DefaultOpen)) 
    {
        bool changedRestDensity = ImGui::DragFloat("Rest Density (p0)", &fluid.RestDensity, 1, 0.1, 100.0);
        ImGui::DragFloat("Stiffness (k)", &fluid.FluidStiffness, 1000, 1000, 10000000);
        ImGui::DragFloat("Viscosity (v)", &fluid.Viscosity, 1000, 0, 10000000);
        bool changedSpacing = ImGui::DragFloat("Particle Spacing (d)", &fluid.ParticleSpacing, 0.1f, 5.0f, 30.0f);

        if (changedSpacing || changedRestDensity) {
            fluid.ParticleMass = fluid.RestDensity * fluid.ParticleSpacing * fluid.ParticleSpacing;
            fluid.SmoothingRadius = 2.50f * fluid.ParticleSpacing;
            
            m_SceneData->PhysicsState->RecalculateBoundaryParticles();
            
            if (!fluid.SimulationRunning)
                m_SceneData->PhysicsState->GridParticleData(fluid.FluidParticleCount, fluid.ParticleSpacing);
        }

        ImGui::BeginDisabled();
        ImGui::DragFloat("Particle Mass (m)", &fluid.ParticleMass, 0.1f, 1.0f, 1000.0f);
        ImGui::DragFloat("Smoothing Radius (h)", &fluid.SmoothingRadius, 1, 20.0f, 50.0, "%.1f");
        ImGui::DragFloat("Gamma Correction 1 (γ1)", &fluid.Gamma1, 0.01f, 0.01f, 1000.0f);
        ImGui::DragFloat("Gamma Correction 2 (γ2)", &fluid.Gamma2, 0.01f, 0.01f, 1000.0f);
        ImGui::EndDisabled();


    }

    if (ImGui::CollapsingHeader("Visualisation", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Match Fluid Particle Size to Spacing", &m_SceneData->MatchBallSizeToSpacing);
        ImGui::Checkbox("Show Pressure Force", &m_SceneData->ShowForceArrow);
        ImGui::Checkbox("Show Kernel Radius", &m_SceneData->ShowKernel);
        ImGui::Checkbox("Show Boundary Particles", &m_SceneData->ShowBoundaryParticles);
        ImGui::DragFloat("Speed Max", &m_SceneData->VMax, 1, 20, 1000);
    }

    ImGui::End();
}

void ImGuiLayer::DrawInspector()
{
    ImGui::Begin("Inspector");

    std::vector<FluidParticle>& particles = m_SceneData->PhysicsState->FluidParticles;
    if (particles.empty()) {
        ImGui::TextDisabled("No particles");
        ImGui::End();
        return;
    }

    int specimenIndex = (int)particles.size() / 2;
    FluidParticle& specimen = particles[specimenIndex];

    ImGui::Text("Particle #%d", specimenIndex);
    ImGui::DragFloat3("Position", glm::value_ptr(specimen.Position), 1, 0.0f, 1000.0f);

    ImGui::BeginDisabled();
    ImGui::InputFloat("Density", &specimen.Density, 0.0f, 0.0f, "%.3f");
    ImGui::InputFloat3("Velocity", glm::value_ptr(specimen.Velocity), "%.3f");
    ImGui::InputFloat("Pressure", &specimen.Pressure, 0.0f, 0.0f, "%.3f");
    ImGui::InputFloat3("Pressure Force Density", glm::value_ptr(specimen.PressureForceDensity), "%.3f");
    ImGui::InputFloat3("Viscous Force Density", glm::value_ptr(specimen.ViscousForceDensity), "%.3f");
    ImGui::EndDisabled();

    ImGui::End();
}

void ImGuiLayer::DrawStats()
{
    ImGuiIO& io = ImGui::GetIO();
    const BoundingBox& viewport = m_SceneData->PhysicsState->BoundaryParameters.Viewport;

    int width, height;
    glfwGetWindowSize(Application::Get()->GetWindow()->Handle(), &width, &height);

    ImGui::Begin("Stats");
    ImGui::Text("%.1f FPS  |  %.3f ms/frame", io.Framerate, 1000.0f / io.Framerate);
    ImGui::Text("Particles: %d", (int)m_SceneData->PhysicsState->FluidParticles.size());
    ImGui::Text("Window: %d x %d  |  Viewport: %.0f x %.0f", width, height, viewport.width(), viewport.height());
    ImGui::End();
}

void ImGuiLayer::BeginFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ImGuiLayer::EndFrame()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLayer::Destroy()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
