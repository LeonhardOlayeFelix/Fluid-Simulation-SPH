#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "debug/Log.h"
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_glfw.h"
#include "vendor/imgui/imgui_impl_opengl3.h"
#include "initialisation/Initialiser.h"
#include "abstraction/Includes.h"

#include "Application.h";


void processInput(GLFWwindow* window);


int main()
{
    ApplicationSpec appSpec;
    appSpec.Title = "Fluid Simulation";
    appSpec.WinSpec.Width = 800;
    appSpec.WinSpec.Height = 600;

    Application app(appSpec);

    app.Run();

        /*GLFWwindow* window = Initialiser::initApplication();

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

        ShaderProgram quadShader("resources/shaders/QuadShader.shader");
        quadShader.SetUniform1i("u_QuadTexture", 0);

        FrameBuffer fbo(800, 600, 4);
        FrameBuffer resolveFbo(800, 600);
        fbo.AddAttachment(AttachmentTarget::Color, AttachmentStorage::Texture);
        fbo.AddAttachment(AttachmentTarget::DepthStencil, AttachmentStorage::RenderBuffer);
        fbo.Validate();
        resolveFbo.AddAttachment(AttachmentTarget::Color, AttachmentStorage::Texture);
        resolveFbo.Validate();



        ImVec2 lastViewPortSize{ 0, 0 };

        while (!glfwWindowShouldClose(window))
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
            ImGui::ShowDemoWindow();

            ImGui::Begin("Viewport");
            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
            if (viewportPanelSize.x != lastViewPortSize.x || viewportPanelSize.y != lastViewPortSize.y)
            {
                lastViewPortSize = viewportPanelSize;
                glViewport(0, 0, (int)viewportPanelSize.x, (int)viewportPanelSize.y);
                fbo.Resize(viewportPanelSize.x, viewportPanelSize.y);
                resolveFbo.Resize(viewportPanelSize.x, viewportPanelSize.y);
            }

            fbo.Bind();
            renderer.Clear();
            renderer.DrawElements(circleVao, shader);
            fbo.Blit(resolveFbo);
            fbo.Unbind();
            renderer.Clear();
            ImGui::Image(reinterpret_cast<void*>(resolveFbo.GetColorTexture().GetId()), viewportPanelSize);
            float aspect = (float)viewportPanelSize.x / viewportPanelSize.y;
            shader.SetUniformMat4f("u_Proj", glm::ortho(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f));
            ImGui::End();


            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwTerminate();
        return 0;*/
}

