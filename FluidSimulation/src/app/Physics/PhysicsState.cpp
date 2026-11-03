#include "PhysicsState.h"
#include <random>

void PhysicsState::RandomiseParticleData(int count)
{

    std::vector<Particle> particles;
    particles.reserve(count);

    for (int i = 0; i < count; i++)
    {
        int xPos = randomIntNormal(0, BoundaryParameters.ViewportSize.x);
        int yPos = randomIntNormal(0, BoundaryParameters.ViewportSize.y);

        particles.emplace_back(glm::vec3{ xPos, yPos, 0 }, glm::vec3{});
    }

    Particles = std::move(particles);
}

void PhysicsState::GridParticleData(int count, float spacing)
{
    std::vector<Particle> particles;
    particles.reserve(count);

    int sideLength = (int)std::ceil(std::sqrt((float)count));

    glm::vec2 centre = BoundaryParameters.ViewportSize / 2.0f;
    float gridWidth = (sideLength - 1) * spacing;
    glm::vec2 origin = centre - glm::vec2(gridWidth / 2.0f);

    for (int i = 0; i < count; i++) {
        int row = i / sideLength;
        int col = i % sideLength;

        glm::vec3 position{ origin.x + col * spacing, origin.y + row * spacing, 0.0f };
        particles.emplace_back(position, glm::vec3{});
    }

    Particles = std::move(particles);
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
