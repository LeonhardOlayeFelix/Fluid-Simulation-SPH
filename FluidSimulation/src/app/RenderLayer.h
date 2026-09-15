#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>

#include "../core/Layer.h"
#include "../core/abstraction/Includes.h"
#include "SceneData.h"
#include "PhysicsSolver.h"



class RenderLayer : public Layer
{
public:
	RenderLayer(std::shared_ptr<SceneData> sceneData);
	virtual ~RenderLayer() override;
	virtual void OnUpdate(double ts) override;
	virtual void OnRender() override;

	void BeginFrame();
	void EndFrame();
	void WriteSceneData();
	void ReadSceneData();

	inline unsigned int GetFrameBufferTextureId() { return m_ResolveFbo.GetColorTexture().GetId(); };
	void OnGuiViewportSizeChanged(int width, int height);

private:
	float getAspectRatio();


private:
	std::shared_ptr<SceneData> m_SceneData;
	BoundaryData m_BoundaryConfig;

	VertexArray m_CircleVao;

	VertexBuffer m_CircleVbo;

	IndexBuffer m_CircleIbo;

	FrameBuffer m_RenderFbo;
	FrameBuffer m_ResolveFbo;

	ShaderProgram m_TestShader;
	Renderer m_Renderer;
};

