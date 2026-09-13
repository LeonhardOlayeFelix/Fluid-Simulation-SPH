#include "Initialiser.h"
#include "../debug/Log.h"
#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_impl_glfw.h"
#include "../vendor/imgui/imgui_impl_opengl3.h"

GLFWwindow* Initialiser::m_Window = nullptr;
bool Initialiser::m_Initialised = false;

GLFWwindow* Initialiser::initApplication()
{
    initLogger();

    m_Window = initGLFW();
    initGLAD();
    initImGui(m_Window);

    m_Initialised = m_Window != nullptr;
    return m_Window;
}

GLFWwindow* Initialiser::GetWindow()
{
    if (!m_Initialised)
        initApplication();

    return m_Window;
}

GLFWwindow* Initialiser::initGLFW()
{
    if (!glfwInit()) {
        SPHCRITICAL("Failed to initialise GLFW");
        return nullptr;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Fluid Simulation", NULL, NULL);

    if (window == NULL) {
        SPHCRITICAL("Failed to create window (GLFW)");
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    return window;
}

void Initialiser::initGLAD()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        SPHCRITICAL("Failed to initialise GLAD");;
        return;
    }
}

void Initialiser::initImGui(GLFWwindow * window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
}

void Initialiser::initLogger()
{
    Log::init();
}


