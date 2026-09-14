#include "Window.h"
#include "debug/Log.h"

Window::Window(WindowSpec winSpec) : m_WinSpec(winSpec)
{

}

void Window::Create()
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    m_Handle = glfwCreateWindow(m_WinSpec.Width, m_WinSpec.Height, "Fluid Simulation", NULL, NULL);

    if (m_Handle == NULL) {
        SPHCRITICAL("Failed to create window (GLFW)");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_Handle);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        SPHCRITICAL("Failed to initialise GLAD");;
        return;
    }
}

void Window::Destroy()
{

    glfwTerminate();
    m_Handle = nullptr;
}

bool Window::ShouldClose()
{
    return glfwWindowShouldClose(m_Handle);
}

void Window::Update()
{
    glfwSwapBuffers(m_Handle);
    glfwPollEvents();
}

GLFWwindow* Window::Handle()
{
    return m_Handle;
}
