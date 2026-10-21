#include "SceneData.h"

void SceneData::SetParticles(std::vector<Particle>& particles)
{
	Particles = std::move(particles);
}
