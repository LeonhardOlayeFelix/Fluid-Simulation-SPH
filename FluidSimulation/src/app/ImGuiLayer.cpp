#include "ImGuiLayer.h"
#include "Application.h"

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
	auto oldSize = m_SceneData->ViewportSize;
	auto currSize = ImGui::GetContentRegionAvail();
	if (currSize.x != oldSize.x || currSize.y != oldSize.y) m_RenderLayer->OnGuiViewportSizeChanged(currSize.x, currSize.y);
	ImGui::Image(reinterpret_cast<void*>(m_SceneData->FramebufferTextureId), currSize);
	ImGui::End();

	//Application Properties
	ImGui::Begin("Application Properties");
	int width, height;
	glfwGetWindowSize(Application::Get()->GetWindow()->Handle(), &width, &height);
	ImGui::Text("Window Size: (%d, %d)", width, height);
	ImGui::Text("Viewport Size: (%g, %g)", m_SceneData->ViewportSize.x, m_SceneData->ViewportSize.y);
	ImGui::Text("Framebuffer Texture Id: %d", m_SceneData->FramebufferTextureId);
	ImGui::Text("Application Frame Rate %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::End();

	//Editor
	ImGui::Begin("Editor");
	ImGui::SliderFloat("Scale", &m_SceneData->Scale, 0, 2);
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
