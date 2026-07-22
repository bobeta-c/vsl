#ifndef STOCHASTIC_SYSTEM_H
#define STOCHASTIC_SYSTEM_H

#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <cmath>
#include <cstdlib>

// Define a type for our potential/distortion functions
typedef float (*PotentialFunction)(Vector3, float);

struct Particle {
    Vector3 basePos; // Position strictly on the base sphere
    Color color;
};

class StochasticSystem {
private:
    std::vector<Particle> particles;
    float temperature;   // The D in our Fokker-Planck equation
    float driftStrength; // How strongly they are pulled up the gradient

    // Generate a random float between -1.0 and 1.0
    float RandomFloat() {
        return -1.0f + 2.0f * ((float)rand() / (float)RAND_MAX);
    }

    // Numerical Gradient: Approximates the uphill direction of your function
    Vector3 ComputeGradient(Vector3 pos, float time, PotentialFunction func) {
        float eps = 0.01f;
        Vector3 grad;
        
        // Central difference approximation for partial derivatives
        grad.x = (func({pos.x + eps, pos.y, pos.z}, time) - func({pos.x - eps, pos.y, pos.z}, time)) / (2.0f * eps);
        grad.y = (func({pos.x, pos.y + eps, pos.z}, time) - func({pos.x, pos.y - eps, pos.z}, time)) / (2.0f * eps);
        grad.z = (func({pos.x, pos.y, pos.z + eps}, time) - func({pos.x, pos.y, pos.z - eps}, time)) / (2.0f * eps);
        
        return grad;
    }

public:
    StochasticSystem(int count, float baseRadius) {
        temperature = 0.5f;
        driftStrength = 2.0f;

        // Initialize particles randomly on the base sphere
        for (int i = 0; i < count; i++) {
            Particle p;
            p.basePos = { RandomFloat(), RandomFloat(), RandomFloat() };
            p.basePos = Vector3Scale(Vector3Normalize(p.basePos), baseRadius);
            
            // Assign a fiery color palette for the particles
            unsigned char r = 200 + rand() % 55;
            unsigned char g = 100 + rand() % 100;
            p.color = { r, g, 50, 255 };
            
            particles.push_back(p);
        }
    }

    // The Euler-Maruyama Integration Step
    void Update(float dt, float time, PotentialFunction currentFunc, float baseRadius) {
        for (auto& p : particles) {
            // 1. Calculate drift (Stochastic Gradient Descent)
            Vector3 drift = ComputeGradient(p.basePos, time, currentFunc);
            
            // 2. Calculate diffusion (Brownian motion)
            float noiseScale = sqrt(2.0f * temperature * dt);
            Vector3 noise = { RandomFloat(), RandomFloat(), RandomFloat() };
            noise = Vector3Scale(noise, noiseScale);

            // 3. Apply Euler-Maruyama: dX = \nabla F dt + \sqrt{2D} dW
            p.basePos.x += drift.x * driftStrength * dt + noise.x;
            p.basePos.y += drift.y * driftStrength * dt + noise.y;
            p.basePos.z += drift.z * driftStrength * dt + noise.z;

            // 4. Project back onto the exact base manifold
            p.basePos = Vector3Scale(Vector3Normalize(p.basePos), baseRadius);
        }
    }

    // Draw the particles exactly on the distorted manifold
    void Draw(float time, PotentialFunction currentFunc, float baseRadius, float amplitude) {
        for (const auto& p : particles) {
            float wave = currentFunc(p.basePos, time);
            float actualRadius = baseRadius + (amplitude * wave);
            
            // Scale the particle out to where the bumpy surface currently is
            Vector3 drawPos = Vector3Scale(Vector3Normalize(p.basePos), actualRadius);
            
            // Draw a tiny sphere for each particle
            DrawSphere(drawPos, 0.08f, p.color);
        }
    }
};

#endif