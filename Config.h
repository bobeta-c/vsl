#ifndef CONFIG_H
#define CONFIG_H

#include "raylib.h" // For Color definitions if needed

namespace Config {
    // Window & Rendering
    constexpr int SCREEN_WIDTH = 1024;
    constexpr int SCREEN_HEIGHT = 768;
    constexpr int TARGET_FPS = 60;
    
    // Manifold Geometry
    constexpr float BASE_RADIUS = 6.0f;
    constexpr float DISTORTION_AMPLITUDE = 0.5f;

    // Particle Rendering
    constexpr float PARTICLE_SIZE = 0.16f;
    
    // Wave Physics
    constexpr float WAVE_FREQUENCY = 1.5f;
    constexpr float WAVE_SPEED = 2.0f;
    constexpr float DROP_FREQUENCY = 12.0f;
    constexpr float DROP_SPEED = 4.0f;
    constexpr float PULSE_WIDTH = 0.1f; // Controls how wide the pulse is
    constexpr float PULSE_HEIGHT = 5.0f; // Controls how tall the pulse is
    
    // Stochastic Engine
    constexpr int PARTICLE_COUNT = 100;
    constexpr float TEMPERATURE = 1.0f;    // Diffusion (D)
    constexpr float DRIFT_STRENGTH = 3.0f; // Gradient pull
    constexpr float GRAD_EPSILON = 0.01f;  // Numerical differentiation step
    constexpr float FRICTION = 0.5f;

    // --- Solarized Dark Theme ---
    // Background: Deep dark blue-gray (base03)
    constexpr Color BG_COLOR = {0, 43, 54, 255};      
    // Wireframe/Base Mesh: Darker gray (base02)
    constexpr Color WIREFRAME_COLOR = {7, 54, 66, 255};  
    // Wave/Distortion Highlights: Cyan
    constexpr Color MANIFOLD_COLOR = {42, 161, 152, 255}; 
    // Particles: Bright Yellow or Magenta for high contrast
    constexpr Color PARTICLE_COLOR = {181, 137, 0, 255};  // Solarized Yellow
    // constexpr Color PARTICLE_COLOR = {211, 54, 130, 255}; // Optional: Solarized Magent
}

#endif