#include "SceneData.h"

void FluidData::SetParticles(std::vector<Particle>& particles)
{
	Particles = std::move(particles);
}
