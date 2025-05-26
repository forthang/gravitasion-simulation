#include "Application.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>
#include <chrono>

static GLFWwindow* window = nullptr;

Application::Application(int width, int height)
    : width(width), height(height), running(false),
      renderer(width, height),
      camera(width, height) {}

Application::~Application() {
    glfwTerminate();
}

bool Application::initWindow() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Gravity Simulator", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // v-sync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return false;
    }

    return true;
}

bool Application::initOpenGL() {
    glViewport(0, 0, width, height);
    return renderer.init();
}

void Application::processInput() {
    float moveSpeed = 10.0f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.move({0, -moveSpeed});
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.move({0, moveSpeed});
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.move({-moveSpeed, 0});
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.move({moveSpeed, 0});
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.zoom(1.05f); // zoom in
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.zoom(0.95f); // zoom out

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        running = false;
}

void Application::mainLoop() {
    float deltaTime = 0.016f;

    while (!glfwWindowShouldClose(window) && running) {
        glfwPollEvents();
        processInput();

        engine.update(deltaTime);
        renderer.render(engine.getBodies(), camera);

        glfwSwapBuffers(window);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void Application::run() {
    if (!initWindow() || !initOpenGL()) {
        return;
    }

    // Демонстрационные тела
    engine.addBody(Body({ 300.0f, 300.0f }, { 0.0f, 0.0f }, 20000.0f));
    engine.addBody(Body({ 500.0f, 300.0f }, { 0.0f, 150.0f }, 5.0f));

    running = true;
    mainLoop();
}
