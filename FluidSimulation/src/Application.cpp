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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    float aspect = (float)800 / 600;

    Renderer renderer;
    Primitive circlePrimitive = Primitives::Circle(40);
    VertexArray vao;
    VertexBuffer vbo(circlePrimitive);
    VertexBufferLayout vbl;
    IndexBuffer ibo(circlePrimitive);
    ShaderProgram shader("resources/shaders/TestShader.shader");

    shader.SetUniformMat4f("u_Model", glm::scale(glm::mat4(1), glm::vec3(0.2)));
    shader.SetUniformMat4f("u_Proj", glm::ortho(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f));

    vbl.push<float>(3);
    vao.RecordVBOLayout(vbo, vbl);
    vao.RecordIndexBuffer(ibo);

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        renderer.Clear();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        //ImGui::ShowDemoWindow();
        
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        aspect = (float)width / height;
        shader.SetUniformMat4f("u_Proj", glm::ortho(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f));

        renderer.DrawElements(vao, shader);

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
