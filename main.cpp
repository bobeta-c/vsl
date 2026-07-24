#include "raylib.h"
#include "raymath.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

// Include our new stochastic engine
#include "Config.h"
#include "StochasticSystem.h" 

// Helper function for a localized mound (Gaussian pulse)
float GaussianMound(Vector3 pos, Vector3 center, float width) {
    float dot = Vector3DotProduct(pos, center);
    // Clamp to prevent NaN errors in acos
    if (dot < -1.0f) dot = -1.0f;
    if (dot > 1.0f) dot = 1.0f;
    float theta = acos(dot); 
    
    // Exponential decay based on distance (bell curve)
    return Config::PULSE_HEIGHT * exp(-(theta * theta) / (2.0f * width * width));
}

// 2: Interfering Moving Pulses
float PulseFunction(Vector3 basePos, float time) {
    Vector3 normal = Vector3Normalize(basePos);
    
    // Controls how wide the mound of water is before it drops off to flat
    float pulseWidth = Config::PULSE_WIDTH; // Narrower means sharper peaks
    
    // Pulse 1: Orbiting flatly around the equator
    Vector3 center1 = { cos(time * 1.5f), 0.0f, sin(time * 1.5f) };
    
    // Pulse 2: Orbiting vertically over the poles
    Vector3 center2 = { 0.0f, cos(time * 1.2f), sin(time * 1.2f) };
    
    // Pulse 3: A tilted orbit moving in reverse
    Vector3 center3 = { cos(-time * 1.0f), sin(-time * 1.0f), 0.0f };
    
    // Calculate the height of each mound at the current vertex
    float wave1 = GaussianMound(normal, center1, pulseWidth);
    float wave2 = GaussianMound(normal, center2, pulseWidth);
    float wave3 = GaussianMound(normal, center3, pulseWidth);
    
    // Superposition: Add them together for constructive interference.
    // We multiply by 0.6f so the sphere doesn't get massively distorted 
    // when all three mounds perfectly intersect.
    return (wave1 + wave2 + wave3) * 0.6f;
}
// 0: Radial Drop (Standing Wave)
float DropFunction(Vector3 basePos, float time) {
    Vector3 dropCenter = { 0.0f, 1.0f, 0.0f }; 
    Vector3 normal = Vector3Normalize(basePos);
    
    float dot = Vector3DotProduct(normal, dropCenter);
    if (dot < -1.0f) dot = -1.0f;
    if (dot > 1.0f) dot = 1.0f;
    
    float theta = acos(dot); 
    
    // Space and time are multiplied. The rings stay in place and pulse.
    return sin(theta * Config::DROP_FREQUENCY) * cos(time * Config::DROP_SPEED);
}

// 1: Superposition Waves (Standing Wave)
float WaveFunction(Vector3 basePos, float time) {
    // The spatial map stays completely stationary
    float spatialX = sin(basePos.x * Config::WAVE_FREQUENCY);
    float spatialY = cos(basePos.y * Config::WAVE_FREQUENCY);
    float spatialZ = sin(basePos.z * Config::WAVE_FREQUENCY);
    
    // The entire map breathes up and down over time
    return (spatialX + spatialY + spatialZ) * cos(time * Config::WAVE_SPEED);
}

float DistortionFunction(Vector3 basePos, float time) {
    return WaveFunction(basePos, time) + DropFunction(basePos, time);
}

float ConstantFunction(Vector3 basePos, float time) {
    return 0.0f; // No distortion
}

float WeirdFunction(Vector3 basePos, float time) {
    return abs(basePos.x);
}
// 3: The Stormy Sea (Turbulent Fractional Brownian Motion)
float StormFunction(Vector3 basePos, float time) {
    float totalDistortion = 0.0f;
    
    float frequency = .2f;  // Starting frequency of the primary waves
    float amplitude = 5.0f;  // Starting height
    float speed = 1.5f;      // Starting wave speed
    
    int octaves = 4;         // How many layers of chaos to add

    for (int i = 0; i < octaves; i++) {
        // Calculate the moving wave on each axis
        float waveX = sin(basePos.x * frequency + time * speed);
        float waveY = sin(basePos.y * frequency + time * (speed * 1.1f)); 
        float waveZ = sin(basePos.z * frequency + time * (speed * 1.2f));
        
        // The absolute value (fabs) is the magic here. 
        // It creates the sharp, non-differentiable "choppy" crests.
        totalDistortion += amplitude * (fabs(waveX) + fabs(waveY) + fabs(waveZ));
        
        // Fractal evolution for the next loop:
        frequency *= 2.0f; // Double the frequency for finer detail
        amplitude *= 0.5f; // Halve the height so the micro-chops don't overpower the main waves
        speed *= 1.3f;     // Speed up the smaller ripples for a boiling effect
    }
    
    // We invert the total and scale it down so it doesn't inflate your sphere too massively.
    // The negative sign ensures the sharp cusps point outward like ocean peaks!
    return -totalDistortion * 0.25f;
}

int main(void) {
    srand(time(NULL)); // Seed the random number generator
    
    InitWindow(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, "Manifold Fokker-Planck - Solarized SGD");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 15.0f, 15.0f, 15.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Mesh sphereMesh = GenMeshSphere(Config::BASE_RADIUS, 64, 64);
    Model sphereModel = LoadModelFromMesh(sphereMesh);
    
    int vertexCount = sphereMesh.vertexCount;
    float* baseVertices = (float*)MemAlloc(vertexCount * 3 * sizeof(float));
    for (int i = 0; i < vertexCount * 3; i++) {
        baseVertices[i] = sphereMesh.vertices[i];
    }

    // Initialize 1000 stochastic particles
    StochasticSystem particleSystem(Config::PARTICLE_COUNT, Config::BASE_RADIUS);

    SetTargetFPS(Config::TARGET_FPS);
    int currentFunctionIndex = 0;

    PotentialFunction potentials[10] = {
    DropFunction,       // Index 0 (Key '0')
    WaveFunction,       // Index 1 (Key '1')
    DistortionFunction,   // Index 2 (Key '2')
    PulseFunction,      // Index 3 (Key '3')
    StormFunction,      // Index 4 (Key '4')
    WeirdFunction,      // Index 5 (Key '5')
    ConstantFunction,   // Index 6 (Key '6')
    ConstantFunction,   // Index 7 (Key '7')
    ConstantFunction,   // Index 8 (Key '8')
    ConstantFunction    // Index 9 (Key '9')
    };

    while (!WindowShouldClose()) {
        float time = GetTime();
        float dt = GetFrameTime(); // Time elapsed since last frame for smooth physics
        
        UpdateCamera(&camera, CAMERA_ORBITAL);


        int key = GetKeyPressed();
        if (key >= KEY_ZERO && key <= KEY_NINE) {
            currentFunctionIndex = key - KEY_ZERO;
        }

        PotentialFunction currentFunc = potentials[currentFunctionIndex];
        
        // Update the manifold geometry
        for (int i = 0; i < vertexCount; i++) {
            Vector3 basePos = { baseVertices[i*3], baseVertices[i*3 + 1], baseVertices[i*3 + 2] };
            float currentRadius = Vector3Length(basePos);
            Vector3 normal = Vector3Scale(basePos, 1.0f / currentRadius);
            
            float wave = currentFunc(basePos, time);
            float newRadius = Config::BASE_RADIUS + (Config::DISTORTION_AMPLITUDE * wave);
            
            Vector3 newPos = Vector3Scale(normal, newRadius);
            
            sphereMesh.vertices[i*3] = newPos.x;
            sphereMesh.vertices[i*3 + 1] = newPos.y;
            sphereMesh.vertices[i*3 + 2] = newPos.z;
        }

        UpdateMeshBuffer(sphereMesh, 0, sphereMesh.vertices, vertexCount * 3 * sizeof(float), 0);

        // Update the physics of the stochastic particles
        particleSystem.Update(dt, time, currentFunc, Config::BASE_RADIUS);

        BeginDrawing();
            ClearBackground(Config::BG_COLOR);
            BeginMode3D(camera);
                
                // Draw the wireframe manifold
                DrawModelWires(sphereModel, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, Config::WIREFRAME_COLOR);
                DrawModel(sphereModel, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, Config::MANIFOLD_COLOR);
                // Draw the diffusing particles
                particleSystem.Draw(time, currentFunc, Config::BASE_RADIUS, Config::DISTORTION_AMPLITUDE);

            EndMode3D();
            
            DrawText(TextFormat("Mode: Function %d (Press number keys to toggle)", currentFunctionIndex), 10, 10, 20, DARKGRAY);
            
            DrawFPS(10, 40);
        EndDrawing();
    }

    MemFree(baseVertices);
    UnloadModel(sphereModel);
    CloseWindow();
    
    return 0;
}