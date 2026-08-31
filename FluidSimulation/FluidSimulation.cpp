#include <glad.h>
#include <glfw3.h>
#include <iostream>

GLFWwindow* initGLFW();
void initGLAD();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

int main()
{
    GLFWwindow* window = initGLFW();
    initGLAD();
    
    glViewport(0, 0, 800, 600);

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

void initGLAD() {

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        return;
    }
}

GLFWwindow* initGLFW() {

    if (!glfwInit()) {
        std::cout << "Failed to initialise GLFW" << std::endl;
        return nullptr;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Fluid Simulation", NULL, NULL);

    if (window == NULL) {
        std::cout << "Failed to create window (GLFW)" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);
    return window;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}