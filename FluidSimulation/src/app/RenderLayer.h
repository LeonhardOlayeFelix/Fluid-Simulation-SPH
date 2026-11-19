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
	void UpdateFluidParticleInstanceBuffers(const std::vector<FluidParticle>& particles);
	void UpdateBoundaryParticleInstanceBuffers(const std::vector<BoundaryParticle>& particles);

	inline unsigned int GetFrameBufferTextureId() { return m_ResolveFbo.GetColorTexture().GetId(); };
	void OnGuiViewportSizeChanged(int width, int height);
	void DrawFluidParticles(const std::vector<FluidParticle>& particles, int specimenIndex, float h);
	void DrawBoundaryParticles(const std::vector<BoundaryParticle>& boundaryParticles);
	void DrawForceArrows(const std::vector<FluidParticle>& particles);
	void DrawKernelRadiusFallOff(glm::vec3 position, float h);
	void DrawKernelRadiusOutline(glm::vec3 position, float h);
	void DrawBoundingBox(BoundingBox box);

private:
	float getAspectRatio();
	float getMaxForceLength(const std::vector<FluidParticle>& particles);

private:
	std::shared_ptr<SceneData> m_SceneData;

	VertexArray m_CircleVao;
	VertexArray m_BoundaryCircleVao;
	VertexArray m_QuadVao;
	VertexArray m_ArrowVao;

	VertexBuffer m_CircleVbo;
	VertexBuffer m_BoundaryCircleVbo;
	VertexBuffer m_QuadVbo;
	VertexBuffer m_ArrowVbo;



	//Instance Vbos
	VertexBuffer m_TranslationsInstanceVbo;
	VertexBuffer m_BoundaryTranslationsInstanceVbo;
	VertexBuffer m_ForceDensityInstanceVbo;
	VertexBuffer m_VelocityInstanceVbo;

	IndexBuffer m_CircleIbo;
	IndexBuffer m_BoundaryCircleIbo;
	IndexBuffer m_QuadIbo;
	IndexBuffer m_ArrowIbo;

	FrameBuffer m_RenderFbo;
	FrameBuffer m_ResolveFbo;

	ShaderProgram m_KernelBallShader;
	ShaderProgram m_ParticleShader;
	ShaderProgram m_RadiusParticleShader;
	ShaderProgram m_VelocityParticleShader;
	ShaderProgram m_KernelRadiusFallOffShader;
	ShaderProgram m_KernelRadiusOutlineShader;
	ShaderProgram m_BoundingBoxOutlineShader;
	ShaderProgram m_WhiteBallShader;
	ShaderProgram m_ArrowShader;

	UniformBuffer m_MatricesUbo;



	std::vector<glm::vec3> m_Translations;
	std::vector<glm::vec3> m_BoundaryTranslations;
	std::vector<glm::vec3> m_ForceDensities;
	std::vector<glm::vec3> m_Velocities;
};

