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

    auto oldSize = m_SceneData->PhysicsState->BoundaryParameters.ViewportSize;
    ImVec2 currSize = ImGui::GetContentRegionAvail();
    if (currSize.x != oldSize.x || currSize.y != oldSize.y)
        m_RenderLayer->OnGuiViewportSizeChanged(currSize.x, currSize.y);

    ImGui::Image(reinterpret_cast<void*>(m_SceneData->FramebufferTextureId), currSize, ImVec2(0, 1), ImVec2(1, 0));

    ImGui::End();
}

void ImGuiLayer::DrawControlsPanel()
{
    PhysicsState& state = *m_SceneData->PhysicsState;
    FluidParameters& fluid = state.FluidParameters;

    ImGui::Begin("Controls");

    if (ImGui::CollapsingHeader("Scene Setup", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderInt("Rows", &m_SceneData->numRows, 1, 30);
        ImGui::SliderInt("Columns", &m_SceneData->numCols, 1, 30);
        if (ImGui::Button("Grid Distribute Particles")) 
            m_SceneData->PhysicsState->GridParticleData(m_SceneData->numRows, m_SceneData->numCols);

        ImGui::SliderInt("Count", &m_SceneData->count, 1, 1000);
        if (ImGui::Button("Normally Distribute Particles")) 
            m_SceneData->PhysicsState->RandomiseParticleData(m_SceneData->count);
    }

    if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen)) {

        if (ImGui::Button(fluid.SimulationRunning ? "Stop Simulation" : "Begin Simulation"))
            fluid.SimulationRunning = !fluid.SimulationRunning;

        ImGui::SliderFloat("Collision Damping", &PhysicsSolver::CollisionDampingFactor, 0.0f, 1.0f, "%.2f");
    }

    if (ImGui::CollapsingHeader("Fluid (SPH)", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Smoothing Radius (h)", &fluid.SmoothingRadius, 0.0f, 1000.0f, "%.1f");
        ImGui::SliderFloat("Alpha", &fluid.Alpha, 1.0f, 200.0f, "%.1f");
    }

    if (ImGui::CollapsingHeader("Visualisation", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Ball Radius (px)", &PhysicsSolver::BallRadius, 0.0f, 500.0f, "%.1f");
        ImGui::Checkbox("Show Kernel", &m_SceneData->showKernel);
        ImGui::Checkbox("Show Density Gradient", &m_SceneData->showDensityGradient);
        if (m_SceneData->showDensityGradient) {
            ImGui::Indent();
            ImGui::Checkbox("Negate Gradient", &m_SceneData->flipDensityGradient);
            ImGui::SliderInt("Arrow Length", &m_SceneData->ArrowLengthMax, 1, 100);
            ImGui::SliderInt("Arrow Width", &m_SceneData->ArrowWidth, 1, 10);
            ImGui::Unindent();
        }
    }

    ImGui::End();
}

void ImGuiLayer::DrawInspector()
{
    ImGui::Begin("Inspector");

    std::vector<Particle>& particles = m_SceneData->PhysicsState->Particles;
    if (particles.empty()) {
        ImGui::TextDisabled("No particles");
        ImGui::End();
        return;
    }

    int specimenIndex = (int)particles.size() / 2;
    Particle& specimen = particles[specimenIndex];

    ImGui::Text("Particle #%d", specimenIndex);
    ImGui::SliderFloat3("Position", glm::value_ptr(specimen.Position), 0.0f, 1000.0f);

    ImGui::BeginDisabled();
    ImGui::InputFloat("Density", &specimen.Density, 0.0f, 0.0f, "%.3f");
    ImGui::InputFloat3("Density Gradient", glm::value_ptr(specimen.DensityGradient), "%.3f");
    ImGui::EndDisabled();

    ImGui::End();
}

void ImGuiLayer::DrawStats()
{
    ImGuiIO& io = ImGui::GetIO();
    const glm::vec2& viewport = m_SceneData->PhysicsState->BoundaryParameters.ViewportSize;

    int width, height;
    glfwGetWindowSize(Application::Get()->GetWindow()->Handle(), &width, &height);

    ImGui::Begin("Stats");
    ImGui::Text("%.1f FPS  |  %.3f ms/frame", io.Framerate, 1000.0f / io.Framerate);
    ImGui::Text("Particles: %d", (int)m_SceneData->PhysicsState->Particles.size());
    ImGui::Text("Window: %d x %d  |  Viewport: %.0f x %.0f", width, height, viewport.x, viewport.y);
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
