#ifndef GRAVITY_SIMULATION_HPP
#define GRAVITY_SIMULATION_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

#include "Shader.hpp"
#include "Camera.hpp"
#include "Object.hpp"
#include "Grid.hpp"

class GravitySimulation {
public:
    GLFWwindow* window = nullptr; 
    bool running = true;         

    GravitySimulation(int width, int height, const char* title);
    ~GravitySimulation();

    GravitySimulation(const GravitySimulation&) = delete;
    GravitySimulation& operator=(const GravitySimulation&) = delete;
    GravitySimulation(GravitySimulation&&) = delete;
    GravitySimulation& operator=(GravitySimulation&&) = delete;

    void run();


    void keyCallback(int key, int scancode, int action, int mods);
    void mouseCallback(double xpos, double ypos);
    void mouseButtonCallback(int button, int action, int mods);
    void scrollCallback(double xoffset, double yoffset);


private:
    Shader* mainShader = nullptr; 
    Camera camera;
    std::vector<Object> objects;
    Grid grid; 

    bool paused = true;
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    float lastMouseX = 400.0f, lastMouseY = 300.0f; 
    bool firstMouse = true;

    bool isCreatingObject = false;

    bool initGLFW(int width, int height, const char* title);
    bool initGLEW();
    void initOpenGLOptions();
    void setupCallbacks();

    void processInput();
    void update();
    void render(const glm::mat4& projection);
};

#endif