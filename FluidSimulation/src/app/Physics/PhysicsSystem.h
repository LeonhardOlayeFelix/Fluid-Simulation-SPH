#pragma once
#include <memory>
#include "PhysicsState.h"

class PhysicsSolver;

class PhysicsSystem
{
public:
	PhysicsSystem(std::shared_ptr<PhysicsState> initialState, double fixedTimeStep = 1.0 / 60.0);

	~PhysicsSystem();

	void OnUpdate(double deltaTime);

	std::shared_ptr<PhysicsState> GetPhysicsState() const;
	void SetFixedTimeStep(double timeStep);
	double GetFixedTimeStep() const { return m_FixedTimeStep; }

private:
	double m_TimeAccumulator = 0.0;
	double m_FixedTimeStep;
	std::shared_ptr<PhysicsState> m_PhysicsState;
	std::shared_ptr<PhysicsSolver> m_Solver;
};
