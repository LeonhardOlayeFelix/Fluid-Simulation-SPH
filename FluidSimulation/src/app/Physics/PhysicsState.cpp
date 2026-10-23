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

void PhysicsState::GridParticleData(int numRows, int numCols)
{

    std::vector<Particle> particles;
    particles.reserve(numRows * numCols);

    int stepY = BoundaryParameters.ViewportSize.y / numRows;
    int stepX = BoundaryParameters.ViewportSize.x / numCols;
    int offsetY = stepY / 2;
    int offsetX = stepX / 2;


    for (int row = 0; row < numRows; row++) {
        for (int col = 0; col < numCols; col++) {
            particles.emplace_back(glm::vec3{ offsetX + col * stepX, offsetY + row * stepY, 0 }, glm::vec3{});
        }
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
