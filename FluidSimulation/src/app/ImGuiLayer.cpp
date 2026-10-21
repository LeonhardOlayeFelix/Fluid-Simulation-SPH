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

	ImGuiIO& io = ImGui::GetIO();

	//Docking
	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

	//Demo Window
	ImGui::ShowDemoWindow();

	//Viewport
	ImGui::Begin("Viewport");
	auto oldSize = m_SceneData->PhysicsState->BoundaryParameters.ViewportSize;
	auto currSize = ImGui::GetContentRegionAvail();
	if (currSize.x != oldSize.x || currSize.y != oldSize.y) m_RenderLayer->OnGuiViewportSizeChanged(currSize.x, currSize.y);
	ImGui::Image(reinterpret_cast<void*>(m_SceneData->FramebufferTextureId), currSize, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::End();

	//Application Properties
	ImGui::Begin("Application Properties");
	int width, height;
	glfwGetWindowSize(Application::Get()->GetWindow()->Handle(), &width, &height);
	ImGui::Text("Window Size: (%d, %d)", width, height);
	ImGui::Text("Viewport Size: (%g, %g)", m_SceneData->PhysicsState->BoundaryParameters.ViewportSize.x, m_SceneData->PhysicsState->BoundaryParameters.ViewportSize.y);
	ImGui::Text("Application Frame Rate %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::End();

	//Physics Solver Properties
	int specimenIndex = m_SceneData->PhysicsState->Particles.size() / 2;

	ImGui::Begin("Physics Solver Properties");
	ImGui::SliderFloat("Collision Damping Factor: %g", &PhysicsSolver::CollisionDampingFactor, 0, 1);
	ImGui::SliderFloat("Ball Radius (px): %g", &PhysicsSolver::BallRadius, 0, 500);
	ImGui::SliderFloat("Smoothing Radius (h): %g", &m_SceneData->PhysicsState->FluidParameters.SmoothingRadius, 0, 1000);

	ImGui::SliderInt("Num rows", &m_SceneData->numRows, 1, 30);
	ImGui::SliderInt("Num cols", &m_SceneData->numCols, 1, 30);

	ImGui::InputFloat("Specimen Density", &m_SceneData->PhysicsState->Particles[specimenIndex].Density, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_ReadOnly);
	ImGui::SliderFloat3("Specimen Position", glm::value_ptr(m_SceneData->PhysicsState->Particles[specimenIndex].Position), 0, 1000);


	if (ImGui::Button((m_SceneData->PhysicsState->FluidParameters.SimulationRunning ? "Stop Simulation" : "Begin Simulation")))
		m_SceneData->PhysicsState->FluidParameters.SimulationRunning = !m_SceneData->PhysicsState->FluidParameters.SimulationRunning;
	ImGui::End();


	EndFrame();
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
