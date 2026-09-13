#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "debug/Log.h"
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_glfw.h"
#include "vendor/imgui/imgui_impl_opengl3.h"
#include "initialisation/Initialiser.h"
#include "abstraction/Includes.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);


int main()
{
    GLFWwindow* window = Initialiser::initApplication();
    glViewport(0, 0, 800, 600);

    std::vector<float> vertices = {
         0.5f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
    };

    std::vector<unsigned int> indicies = {
        0, 1, 2,
        0, 2, 3
    };

    VertexArray vao;
    VertexBuffer vbo(vertices);
    VertexBufferLayout vbl;
    IndexBuffer ibo(indicies);
    ShaderProgram shader("resources/shaders/TestShader.shader");

    vbl.push<float>(3);
    vao.RecordVBOLayout(vbo, vbl);
    vao.RecordIndexBuffer(ibo);

    Renderer renderer;



    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        renderer.Clear();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();

        renderer.DrawArray(vao, shader);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
