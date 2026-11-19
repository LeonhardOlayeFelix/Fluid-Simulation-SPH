#include "PhysicsState.h"
#include <random>

void PhysicsState::RandomiseParticleData(int count)
{

    std::vector<FluidParticle> particles;
    particles.reserve(count);

    for (int i = 0; i < count; i++)
    {
        int xPos = randomIntNormal(0, BoundaryParameters.Viewport.width());
        int yPos = randomIntNormal(0, BoundaryParameters.Viewport.height());

        particles.emplace_back(glm::vec3{ xPos, yPos, 0 }, glm::vec3{});
    }

    FluidParticles = std::move(particles);
}

void PhysicsState::GridParticleData(int count, float spacing)
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

void PhysicsState::RecalculateBoundaryParticles()
{
    BoundingBox domain = BoundaryParameters.ParticleDomain;

    float d = FluidParameters.ParticleSpacing;

    BoundaryParticles.clear();

    int nx = std::max(1, (int)std::round(domain.width() / d));
    int ny = std::max(1, (int)std::round(domain.height() / d));

    float dx = domain.width() / nx;
    float dy = domain.height() / ny;

    BoundaryParticles.reserve(2 * (nx + 1) + 2 * (ny - 1));

    for (int i = 0; i <= nx; i++) {
        float x = domain.xmin() + i * dx;
        BoundaryParticles.push_back(BoundaryParticle({ x, domain.ymin(), 0 }));
        BoundaryParticles.push_back(BoundaryParticle({ x, domain.ymax(), 0 }));
    }

    for (int j = 1; j < ny; j++) {
        float y = domain.ymin() + j * dy;
        BoundaryParticles.push_back(BoundaryParticle({ domain.xmin(), y, 0 }));
        BoundaryParticles.push_back(BoundaryParticle({ domain.xmax(), y, 0 }));
    }

    BoundaryParameters.Stale = true;

}

int PhysicsState::randomInt(int min, int max)
{
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

int PhysicsState::randomIntNormal(int min, int max)
{
    static std::mt19937 rng(std::random_device{}());
    float mean = 0.5f * (min + max);
    float stddev = (max - min) / 9.0f;
    std::normal_distribution<float> dist(mean, stddev);
    int value = (int)std::round(dist(rng));
    return std::clamp(value, min, max);
}
