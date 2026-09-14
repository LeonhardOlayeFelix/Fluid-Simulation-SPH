#include "Application.h"

#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_impl_glfw.h"
#include "../vendor/imgui/imgui_impl_opengl3.h"

#include "../core/Log.h"

Application* Application::s_Application = nullptr;

Application::Application(ApplicationSpec appSpec) : m_AppSpec(appSpec)
{
    s_Application = this;

    initLogger();
    initWindow();
    initImGui();
    m_SceneData = std::make_shared<SceneData>();
    m_RenderLayer = std::make_shared<RenderLayer>(m_SceneData);
    m_ImGuiLayer = std::make_shared<ImGuiLayer>(m_SceneData, m_RenderLayer);
}

void Application::Run()
{
    double lastFrame = 0.0f;

    while (!m_Window->ShouldClose()) {

        double timeStep = GetTime() - lastFrame;


        m_ImGuiLayer->OnUpdate(timeStep);
        m_RenderLayer->OnUpdate(timeStep);

        glClear(GL_COLOR_BUFFER_BIT);

        m_ImGuiLayer->OnRender();
        m_RenderLayer->OnRender();

        m_Window->Update();

    }
}

Application* Application::Get()
{
    return s_Application;
}

double Application::GetTime()
{
    return glfwGetTime();
}

void Application::initLogger()
{
	Log::init();
}

void Application::initWindow()
{
    if (!glfwInit()) {
        SPHCRITICAL("Failed to initialise GLFW");
        return;
    }

    m_Window = std::make_shared<Window>(m_AppSpec.WinSpec);
    m_Window->Create();
}

void Application::initImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(m_Window->Handle(), true);
    ImGui_ImplOpenGL3_Init();
}
