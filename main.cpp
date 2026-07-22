#include "raylib.h"
#include "raymath.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

// Include our new stochastic engine
#include "StochasticSystem.h" 

float WaveFunction(Vector3 basePos, float time) {
    float frequency = 1.5f;
    float speed = 2.0f;
    float waveX = sin(basePos.x * frequency + time * speed);
    float waveY = cos(basePos.y * frequency + time * speed);
    float waveZ = sin(basePos.z * frequency + time * speed);
    return waveX + waveY + waveZ;
}

float DropFunction(Vector3 basePos, float time) {
    Vector3 dropCenter = { 0.0f, 1.0f, 0.0f }; 
    Vector3 normal = Vector3Normalize(basePos);
    
    float dot = Vector3DotProduct(normal, dropCenter);
    if (dot < -1.0f) dot = -1.0f;
    if (dot > 1.0f) dot = 1.0f;
    
    float theta = acos(dot); 
    float frequency = 12.0f; 
    float speed = 4.0f;      
    
    return sin(theta * frequency - time * speed);
}

int main(void) {
    srand(time(NULL)); // Seed the random number generator
    
    const int screenWidth = 1024;
    const int screenHeight = 768;
    InitWindow(screenWidth, screenHeight, "Manifold Fokker-Planck Visualizer");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 15.0f, 15.0f, 15.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    float baseRadius = 5.0f;
    Mesh sphereMesh = GenMeshSphere(baseRadius, 64, 64);
    Model sphereModel = LoadModelFromMesh(sphereMesh);
    
    int vertexCount = sphereMesh.vertexCount;
    float* baseVertices = (float*)MemAlloc(vertexCount * 3 * sizeof(float));
    for (int i = 0; i < vertexCount * 3; i++) {
        baseVertices[i] = sphereMesh.vertices[i];
    }

    // Initialize 1000 stochastic particles
    StochasticSystem particleSystem(1000, baseRadius);

    SetTargetFPS(60);
    bool useDrop = true;

    while (!WindowShouldClose()) {
        float time = GetTime();
        float dt = GetFrameTime(); // Time elapsed since last frame for smooth physics
        
        UpdateCamera(&camera, CAMERA_ORBITAL);

        if (IsKeyPressed(KEY_SPACE)) {
            useDrop = !useDrop;
        }

        // Determine which function is currently active
        PotentialFunction currentFunc = useDrop ? DropFunction : WaveFunction;
        float distortionAmplitude = 0.5f; 
        
        // Update the manifold geometry
        for (int i = 0; i < vertexCount; i++) {
            Vector3 basePos = { baseVertices[i*3], baseVertices[i*3 + 1], baseVertices[i*3 + 2] };
            float currentRadius = Vector3Length(basePos);
            Vector3 normal = Vector3Scale(basePos, 1.0f / currentRadius);
            
            float wave = currentFunc(basePos, time);
            float newRadius = baseRadius + (distortionAmplitude * wave);
            
            Vector3 newPos = Vector3Scale(normal, newRadius);
            
            sphereMesh.vertices[i*3] = newPos.x;
            sphereMesh.vertices[i*3 + 1] = newPos.y;
            sphereMesh.vertices[i*3 + 2] = newPos.z;
        }

        UpdateMeshBuffer(sphereMesh, 0, sphereMesh.vertices, vertexCount * 3 * sizeof(float), 0);

        // Update the physics of the stochastic particles
        particleSystem.Update(dt, time, currentFunc, baseRadius);

        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
                
                // Draw the wireframe manifold
                DrawModelWires(sphereModel, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, {0, 50, 150, 100});
                
                // Draw the diffusing particles
                particleSystem.Draw(time, currentFunc, baseRadius, distortionAmplitude);

            EndMode3D();
            
            if (useDrop) DrawText("Mode: Drop Function (Press SPACE to toggle)", 10, 10, 20, DARKGRAY);
            else DrawText("Mode: Wave Function (Press SPACE to toggle)", 10, 10, 20, DARKGRAY);
            
            DrawFPS(10, 40);
        EndDrawing();
    }

    MemFree(baseVertices);
    UnloadModel(sphereModel);
    CloseWindow();
    
    return 0;
}