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
	void gridParticleData();
	void randomiseParticleData();
	int randomInt(int min, int max);
	int randomIntNormal(int min, int max);

private:
	std::shared_ptr<SceneData> m_SceneData;

	VertexArray m_CircleVao;
	VertexArray m_QuadVao;

	VertexBuffer m_CircleVbo;
	VertexBuffer m_QuadVbo;

	IndexBuffer m_CircleIbo;
	IndexBuffer m_QuadIbo;

	FrameBuffer m_RenderFbo;
	FrameBuffer m_ResolveFbo;

	ShaderProgram m_BallShader;
	ShaderProgram m_KernelShader;
};

