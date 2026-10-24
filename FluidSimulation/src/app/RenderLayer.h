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
	void UpdateInstanceBuffers(const std::vector<Particle>& particles);

	inline unsigned int GetFrameBufferTextureId() { return m_ResolveFbo.GetColorTexture().GetId(); };
	void OnGuiViewportSizeChanged(int width, int height);
	void DrawParticles(const std::vector<Particle>& particles, int specimenIndex, float h);
	void DrawDensityGradients(const std::vector<Particle>& particles);
	void DrawKernelRadius(glm::vec3 position, float h);

private:
	float getAspectRatio();
	float getMaxDensityGradient(const std::vector<Particle>& particles);

private:
	std::shared_ptr<SceneData> m_SceneData;

	VertexArray m_CircleVao;
	VertexArray m_QuadVao;
	VertexArray m_ArrowVao;

	VertexBuffer m_CircleVbo;
	VertexBuffer m_QuadVbo;
	VertexBuffer m_ArrowVbo;

	//Instance Vbos
	VertexBuffer m_TranslationsInstanceVbo;
	VertexBuffer m_DensityGradientsInstanceVbo;

	IndexBuffer m_CircleIbo;
	IndexBuffer m_QuadIbo;
	IndexBuffer m_ArrowIbo;

	FrameBuffer m_RenderFbo;
	FrameBuffer m_ResolveFbo;

	ShaderProgram m_KernelBallShader;
	ShaderProgram m_DefaultBallShader;
	ShaderProgram m_KernelRadiusShader;
	ShaderProgram m_ArrowShader;

	UniformBuffer m_MatricesUbo;
};

