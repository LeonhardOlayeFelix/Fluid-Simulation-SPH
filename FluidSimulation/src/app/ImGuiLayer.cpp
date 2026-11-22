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
	DrawControlsPanels();
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

void ImGuiLayer::DrawControlsPanels()
{


    DrawSimulationControlsControlPanel();
    DrawVisualisationControlPanel();

    
}

void ImGuiLayer::DrawSimulationControlsControlPanel()
{
    ImGui::Begin("Simulation Controls");
    ImGui::PushItemWidth(100);

    PhysicsState& state = *m_SceneData->PhysicsState;
    FluidParameters& fluid = state.FluidParameters;



    //scene setup: initial particle layout
    if (ImGui::CollapsingHeader("Setup", ImGuiTreeNodeFlags_DefaultOpen))
    {
        bool a = ImGui::DragInt("Count", &fluid.FluidParticleCount, 1.0, 1, 10000);

        if (a && !fluid.SimulationRunning)
            state.GridParticleData(fluid.FluidParticleCount, fluid.ParticleSpacing);

        if (ImGui::Button("Set"))
            state.GridParticleData(fluid.FluidParticleCount, fluid.ParticleSpacing);
    }

    //fluid's physical parameters
    if (ImGui::CollapsingHeader("Fluid (SPH)", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::SeparatorText("Influences");
        ImGui::Checkbox("Influenced by Gravity", &fluid.InfluencedByGravity);
        ImGui::Checkbox("Influenced by Pressure", &fluid.InfluencedByPressure);
        ImGui::Checkbox("Influenced by Surface Tension", &fluid.InfluencedBySurfaceTension);
        ImGui::Checkbox("Influenced by Boundary Particles", &fluid.InfluencedByBoundaryParticles);
        ImGui::Checkbox("Influenced by Viscous Force", &fluid.InfluencedByViscousForce);


        ImGui::SeparatorText("Coefficients");
        ImGui::DragFloat("Stiffness (k)", &fluid.Stiffness, 1000, 1000, 10000000);
        ImGui::DragFloat("Viscosity (v)", &fluid.Viscosity, 1000, 0, 10000000);
        ImGui::DragFloat("Tension (o)", &fluid.TensionCoefficient, 1, 0, 10000000);

        ImGui::SeparatorText("Other ");
        bool changedRestDensity = ImGui::DragFloat("Rest Density (p0)", &fluid.RestDensity, 1, 1.0, 100.0);
        bool changedSpacing = ImGui::DragFloat("Particle Spacing (d)", &fluid.ParticleSpacing, 0.1f, 5.0f, 30.0f);
        ImGui::DragFloat("Surface Tension Threshold (l)", &fluid.SurfaceTensionThreshold, 0.0001, 0.0001, 0.1);
        ImGui::Checkbox("Clamp Pressure (>0)", &fluid.ClampPressure);

        if (changedSpacing || changedRestDensity)
        {
            fluid.ParticleMass = fluid.RestDensity * fluid.ParticleSpacing * fluid.ParticleSpacing;
            fluid.SmoothingRadius = 2.50f * fluid.ParticleSpacing;

            state.RecalculateBoundaryParticles();

            if (!fluid.SimulationRunning)
                state.GridParticleData(fluid.FluidParticleCount, fluid.ParticleSpacing);
        }

        ImGui::SeparatorText("Derived");
        ImGui::BeginDisabled();
        ImGui::DragFloat("Particle Mass (m)", &fluid.ParticleMass);
        ImGui::DragFloat("Smoothing Radius (h)", &fluid.SmoothingRadius, 1, 0, 0, "%.1f");
        ImGui::DragFloat("Gamma Correction 1 (γ1)", &fluid.Gamma1);
        ImGui::DragFloat("Gamma Correction 2 (γ2)", &fluid.Gamma2);
        ImGui::EndDisabled();
    }

    //simulation
    if (ImGui::CollapsingHeader("State", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button(fluid.SimulationRunning ? "Pause Simulation" : "Begin Simulation"))
            fluid.SimulationRunning = !fluid.SimulationRunning;
    }

    ImGui::PopItemWidth();
    ImGui::End();



}

void ImGuiLayer::DrawVisualisationControlPanel()
{
    //visualisation... no effect on physics
    ImGui::Begin("Visualisation Controls");
    ImGui::PushItemWidth(100);
    if (ImGui::CollapsingHeader("Visualisation", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::SeparatorText("Fluid Particles");
        ImGui::Checkbox("Match Fluid Particle Size to Spacing", &m_SceneData->MatchBallSizeToSpacing);
        ImGui::BeginDisabled(m_SceneData->MatchBallSizeToSpacing);
        ImGui::DragFloat("Fluid Particle Radius", &PhysicsSolver::FluidParticleRadius, 0.1f, 0.1f, 50.0f);
        ImGui::EndDisabled();

        ImGui::SeparatorText("Overlays");
        ImGui::Checkbox("Show Pressure Force", &m_SceneData->ShowForceArrow);
        ImGui::Checkbox("Show Kernel Radius", &m_SceneData->ShowKernel);

        ImGui::SeparatorText("Boundary");
        ImGui::Checkbox("Show Boundary", &m_SceneData->ShowParticleDomain);
        ImGui::Checkbox("Show Boundary Particles", &m_SceneData->ShowBoundaryParticles);
        if (m_SceneData->ShowBoundaryParticles)
            ImGui::DragFloat("Boundary Particle Radius", &PhysicsSolver::BoundaryParticleRadius, 0.1f, 0.1f, 50.0f);

        ImGui::SeparatorText("Shading");
        static const char* particleShaderNames[] = { "Plain Shader", "Velocity Shader", "Surface Tension Shader" };

        ImGui::PushItemWidth(200);
        int shaderIndex = (int)m_SceneData->FluidParticleShader;
        if (ImGui::Combo("Fluid Shader", &shaderIndex, particleShaderNames, IM_ARRAYSIZE(particleShaderNames)))
            m_SceneData->FluidParticleShader = (FluidParticleShaderTypes)shaderIndex;
        ImGui::PopItemWidth();

        if (shaderIndex == (int)FluidParticleShaderTypes::VelocityShader)
            ImGui::DragFloat("Speed Max", &m_SceneData->VMax, 1, 20, 1000);


    }
    ImGui::PopItemWidth();
    ImGui::End();
}

void ImGuiLayer::DrawInspector()
{
    ImGui::Begin("Particle Inspector");

    std::vector<FluidParticle>& particles = m_SceneData->PhysicsState->FluidParticles;
    if (particles.empty()) {
        ImGui::TextDisabled("No particles");
        ImGui::End();
        return;
    }

    int specimenIndex = (int)particles.size() / 2;
    FluidParticle& specimen = particles[specimenIndex];

    ImGui::PushItemWidth(200);

    ImGui::Text("Particle #%d", specimenIndex);

    //kinematics
    ImGui::SeparatorText("Kinematics");
    ImGui::DragFloat2("Position", glm::value_ptr(specimen.Position), 1, 0.0f, 1000.0f);
    ImGui::BeginDisabled();
    ImGui::InputFloat2("Velocity", glm::value_ptr(specimen.Velocity), "%.3f");
    ImGui::InputFloat2("Acceleration", glm::value_ptr(specimen.Acceleration), "%.3f");
    ImGui::EndDisabled();

    //sph scalar quantities
    ImGui::SeparatorText("SPH Scalar Fields");
    ImGui::BeginDisabled();
    ImGui::InputFloat("Density", &specimen.Density, 0.0f, 0.0f, "%.3f");
    ImGui::InputFloat("Pressure", &specimen.Pressure, 0.0f, 0.0f, "%.3f");
    ImGui::EndDisabled();

    //sph vector quantites
    ImGui::SeparatorText("SPH Vector Fields");
    ImGui::BeginDisabled();
    ImGui::InputFloat2("Pressure Force Density", glm::value_ptr(specimen.PressureForceDensity), "%.3f");
    ImGui::InputFloat2("Viscous Force Density", glm::value_ptr(specimen.ViscousForceDensity), "%.3f");
    ImGui::InputFloat2("Color Field Gradient", glm::value_ptr(specimen.ColorFieldGradient), "%.3f");
    ImGui::EndDisabled();

    ImGui::PopItemWidth();
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
