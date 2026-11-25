#include "PhysicsData.h"
#include <random>

void PhysicsData::GridParticleData(int count, float spacing)
{
    std::vector<FluidParticle> particles;
    particles.reserve(count);

    int sideLength = (int)std::ceil(std::sqrt((float)count));

    glm::vec2 centre = BoundaryParameters.ParticleDomain.centre();
    float gridWidth = (sideLength - 1) * spacing;
    glm::vec2 origin = centre - glm::vec2(gridWidth / 2.0f);

    for (int i = 0; i < count; i++) {
        int row = i / sideLength;
        int col = i % sideLength;

        glm::vec3 position{ origin.x + col * spacing, origin.y + row * spacing, 0.0f };
        particles.emplace_back(position, glm::vec3{});
    }

    FluidParticles = std::move(particles);
}

void PhysicsData::RecalculateBoundaryParticles()
{
    float d = FluidParameters.ParticleSpacing;
    float offset = d / 2;

    BoundingBox domain = BoundaryParameters.ParticleDomain;
    
    BoundingBox surfaceToSample = BoundingBox(domain.min() - glm::vec2{ offset, offset }, domain.max() + glm::vec2{ offset, offset });

    BoundaryParticles.clear();

    int nx = std::max(1, (int)std::round(surfaceToSample.width() / d));
    int ny = std::max(1, (int)std::round(surfaceToSample.height() / d));

    float dx = surfaceToSample.width() / nx;
    float dy = surfaceToSample.height() / ny;

    BoundaryParticles.reserve(2 * (nx + 1) + 2 * (ny - 1));

    for (int i = 0; i <= nx; i++) {
        float x = surfaceToSample.xmin() + i * dx;
        BoundaryParticles.push_back(BoundaryParticle({ x, surfaceToSample.ymin(), 0 }));
        BoundaryParticles.push_back(BoundaryParticle({ x, surfaceToSample.ymax(), 0 }));
    }

    for (int j = 1; j < ny; j++) {
        float y = surfaceToSample.ymin() + j * dy;
        BoundaryParticles.push_back(BoundaryParticle({ surfaceToSample.xmin(), y, 0 }));
        BoundaryParticles.push_back(BoundaryParticle({ surfaceToSample.xmax(), y, 0 }));
    }

    BoundaryParameters.Stale = true;
}
