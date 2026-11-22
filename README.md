# (In-Progress) Fluid Simulation SPH

A real-time fluid simulation using **Smoothed Particle Hydrodynamics (SPH)**, written in C++ and OpenGL.

<img width="1916" height="1030" alt="image" src="https://github.com/user-attachments/assets/2b5896bb-742f-4e47-82ef-4705d7a2d103" />


> **Status:** in development.

## What is SPH?

SPH represents a fluid as a set of particles. Any field quantity $A$ (such as density or pressure) at a position $\mathbf{r}$ is found by summing contributions from nearby particles, weighted by a **smoothing kernel** $W$, a function that falls to zero at the smoothing radius $h$:

$$A(\mathbf{r}) = \sum_j m_j \, \frac{A_j}{\rho_j} \, W(\mathbf{r} - \mathbf{r}_j, h)$$

where $m_j$, $\rho_j$, $\mathbf{r}_j$ and $A_j$ are the mass, density, position and value of $A$ for particle $j$. Gradients of a field come from the same sum with $W$ replaced by its gradient $\nabla W$.

This project follows Müller, Charypar & Gross (2003), *Particle-Based Fluid Simulation for Interactive Applications*, along with various other papers used for rendering not yet mentioned here.

## Building

Windows only. Requires **Visual Studio 2026** (toolset v145) and **CMake**.

1. Clone with submodules:
   ```bash
   git clone --recursive https://github.com/LeonhardOlayeFelix/Fluid-Simulation-SPH.git
   ```
2. Build GLFW:
   ```bash
   cmake -S Dependencies/glfw -B Dependencies/glfw/build
   cmake --build Dependencies/glfw/build --config Debug
   ```
   Use `--config Release` as well if you want to build the Release configuration.
3. Open `FluidSimulation.slnx` and run.

GLAD and GLM are included in the repo. Dear ImGui (docking branch) and spdlog come in as submodules.

## References

- M. Müller, D. Charypar, M. Gross. *Particle-Based Fluid Simulation for Interactive Applications*. SCA 2003, along with various other papers used for rendering not yet mentioned here.

## Acknowledgements

[GLFW](https://www.glfw.org), [GLAD](https://github.com/Dav1dde/glad), [GLM](https://github.com/g-truc/glm), [Dear ImGui](https://github.com/ocornut/imgui), [spdlog](https://github.com/gabime/spdlog)
