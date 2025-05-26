#pragma once

#include "PhysicsEngine.hpp"
#include "Renderer.hpp"
#include "Camera.hpp"

class Application {
public:
    Application(int width, int height);
    ~Application();
    void run();

private:
    int width, height;
    bool running;

    PhysicsEngine engine;
    Renderer renderer;
    Camera camera;

    bool initWindow();
    bool initOpenGL();
    void processInput();
    void mainLoop();
};
