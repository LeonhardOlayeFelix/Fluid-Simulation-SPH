#include "ImGuiLayer.h"
#include "Application.h"
#include "PhysicsSolver.h"
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
	auto oldSize = m_SceneData->BoundaryData.ViewportSize;
	auto currSize = ImGui::GetContentRegionAvail();
	if (currSize.x != oldSize.x || currSize.y != oldSize.y) m_RenderLayer->OnGuiViewportSizeChanged(currSize.x, currSize.y);
	ImGui::Image(reinterpret_cast<void*>(m_SceneData->FramebufferTextureId), currSize, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::End();

	//Application Properties
	ImGui::Begin("Application Properties");
	int width, height;
	glfwGetWindowSize(Application::Get()->GetWindow()->Handle(), &width, &height);
	ImGui::Text("Window Size: (%d, %d)", width, height);
	ImGui::Text("Viewport Size: (%g, %g)", m_SceneData->BoundaryData.ViewportSize.x, m_SceneData->BoundaryData.ViewportSize.y);
	ImGui::Text("Application Frame Rate %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::End();

	//Editor
	ImGui::Begin("Editor");
	ImGui::Text("Gravity (px/s²): %g", PhysicsSolver::Gravity);
	ImGui::Text("Velocity (px/s²): (%g, %g, %g)", m_SceneData->FluidData.Particles[0].Velocity.x, m_SceneData->FluidData.Particles[0].Velocity.y, m_SceneData->FluidData.Particles[0].Velocity.z);

	if (ImGui::Button("Reset particle")) {
		m_SceneData->FluidData.Particles[0].Velocity.y += 400;
		m_SceneData->FluidData.Particles[0].Velocity.x += 400;
	}
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
