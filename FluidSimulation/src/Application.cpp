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
    VertexArray circleVao;
    VertexBuffer circleVbo(circlePrimitive);
    VertexBufferLayout circleVbl;
    IndexBuffer circleIbo(circlePrimitive);
    circleVbl.push<float>(3);
    circleVao.RecordVBOLayout(circleVbo, circleVbl);
    circleVao.RecordIndexBuffer(circleIbo);

    VertexArray quadVao;
    Primitive quadPrimitive = Primitives::NDCQuad();
    VertexBuffer quadVbo(quadPrimitive);
    IndexBuffer quadIbo(quadPrimitive);
    VertexBufferLayout quadVbl;
    quadVbl.push<float>(3);
    quadVbl.push<float>(2);
    quadVao.RecordVBOLayout(quadVbo, quadVbl);
    quadVao.RecordIndexBuffer(quadIbo);

    ShaderProgram shader("resources/shaders/TestShader.shader");
    shader.SetUniformMat4f("u_Model", glm::scale(glm::mat4(1), glm::vec3(0.2)));
    shader.SetUniformMat4f("u_Proj", glm::ortho(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f));

    ShaderProgram quadShader("resources/shaders/QuadShader.shader");
    quadShader.SetUniform1i("u_QuadTexture", 0);

    FrameBuffer fbo(800, 600, 4);
    FrameBuffer resolveFbo(800, 600);
    fbo.AddAttachment(AttachmentTarget::Color, AttachmentStorage::Texture);
    fbo.AddAttachment(AttachmentTarget::DepthStencil, AttachmentStorage::RenderBuffer);
    fbo.Validate();
    resolveFbo.AddAttachment(AttachmentTarget::Color, AttachmentStorage::Texture);
    resolveFbo.Validate();





    while (!glfwWindowShouldClose(window))
    {
        processInput(window);


        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::ShowDemoWindow();

        fbo.Bind();
        renderer.Clear();
        renderer.DrawElements(circleVao, shader);
        fbo.Blit(resolveFbo);
        fbo.Unbind();

        renderer.Clear();

        resolveFbo.GetColorTexture().Bind(0);
        quadShader.SetUniform1i("u_QuadTexture", 0);
        glDisable(GL_DEPTH_TEST);
        renderer.DrawElements(quadVao, quadShader);
         
        //renderer.Clear();
        //renderer.DrawElements(circleVao, shader);

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
