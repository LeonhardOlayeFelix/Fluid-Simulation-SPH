#pragma once
#include <memory>
#include "../../core/Layer.h"
#include "../Data/SceneData.h"

class PhysicsSolver;

class PhysicsLayer : public Layer
{
public:
	PhysicsLayer(std::shared_ptr<PhysicsData> initialState, double fixedTimeStep = 1.0 / 160.0);

	~PhysicsLayer();

	void OnUpdate(double deltaTime);

	std::shared_ptr<PhysicsData> GetPhysicsState() const;
	void SetFixedTimeStep(double timeStep);
	double GetFixedTimeStep() const { return m_FixedTimeStep; }

private:
	double m_TimeAccumulator = 0.0;
	double m_FixedTimeStep;
	std::shared_ptr<PhysicsData> m_PhysicsState;
	std::shared_ptr<PhysicsSolver> m_Solver;
};
