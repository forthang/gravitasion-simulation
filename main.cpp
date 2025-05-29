#include "globals.h"
#include "callbacks.h"
#include "shaders.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

// Инициализация глобальных переменных
namespace Global {
    std::vector<PhysicsObject> objects;
    Camera camera(glm::vec3(0.0f, 1000.0f, 5000.0f));
    Grid* grid = nullptr;
    GLFWwindow* window = nullptr;
    GLuint shaderProgram;
    bool running = true;
    bool pause = true;
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
}

using namespace Global;

#include "globals.h"
#include "callbacks.h"
#include "shaders.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

// Инициализация глобальных переменных
namespace Global {
    std::vector<PhysicsObject> objects;
    Camera camera(glm::vec3(0.0f, 1000.0f, 5000.0f));
    Grid* grid = nullptr;
    GLFWwindow* window = nullptr;
    GLuint shaderProgram;
    bool running = true;
    bool pause = true;
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
}

using namespace Global;

GLuint CreateShaderProgram(const char* vertexSource, const char* fragmentSource) {
    // Реализация из исходного кода пользователя
    // ...
}

void InitializeGLFW() {
    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    window = glfwCreateWindow(800, 600, "Gravity Simulator", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        std::cerr << "Window creation failed!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void InitializeGLEW() {
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialization failed!" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
}

void SetupScene() {
    // Инициализация объектов
    objects = {
        PhysicsObject(glm::vec3(3844, 0, 0), glm::vec3(0, 0, 600), 7.34767309e21f),
        PhysicsObject(glm::vec3(-3000, 650, -350), glm::vec3(0, 0, 800), 5.97219e22f,
                    5515.0f, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)),
        PhysicsObject(glm::vec3(0, 0, -350), glm::vec3(0), 1.989e25f, 
                    5515.0f, glm::vec4(1.0f, 0.929f, 0.176f, 1.0f), true)
    };

    // Создание сетки
    grid = new Grid(20000.0f, 25);

    // Настройка шейдеров
    shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
    glUseProgram(shaderProgram);

    // Настройка проекции
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f), 
        800.0f/600.0f, 
        0.1f, 
        750000.0f
    );
    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "projection"),
        1, 
        GL_FALSE, 
        glm::value_ptr(projection)
    );
}

void Cleanup() {
    delete grid;
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
}

int main() {
    InitializeGLFW();
    InitializeGLEW();
    
    // Настройка OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, 800, 600);

    SetupScene();

    // Основной цикл
    while (!glfwWindowShouldClose(window) && running) {
        // Расчет времени
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Обновление
        camera.updateViewMatrix(shaderProgram);
        grid->Update(objects);

        // Физика
        if (!pause) {
            for (auto& obj : objects) {
                obj.UpdatePosition(false);
            }
        }

        // Отрисовка
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Отрисовка сетки
        GLint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
        glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 0.25f);
        glUniform1i(glGetUniformLocation(shaderProgram, "isGrid"), 1);
        grid->Draw(shaderProgram);

        // Отрисовка объектов
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniform1i(glGetUniformLocation(shaderProgram, "isGrid"), 0);
        
        for (auto& obj : objects) {
            glUniform4f(colorLoc, 
                      obj.GetColor().r, 
                      obj.GetColor().g, 
                      obj.GetColor().b, 
                      obj.GetColor().a);
            obj.Draw(shaderProgram);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Cleanup();
    return 0;
}

void InitializeGLFW() {
    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    window = glfwCreateWindow(800, 600, "Gravity Simulator", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        std::cerr << "Window creation failed!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void InitializeGLEW() {
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialization failed!" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
}

void SetupScene() {
    // Инициализация объектов
    objects = {
        PhysicsObject(glm::vec3(3844, 0, 0), glm::vec3(0, 0, 600), 7.34767309e21f),
        PhysicsObject(glm::vec3(-3000, 650, -350), glm::vec3(0, 0, 800), 5.97219e22f,
                    5515.0f, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)),
        PhysicsObject(glm::vec3(0, 0, -350), glm::vec3(0), 1.989e25f, 
                    5515.0f, glm::vec4(1.0f, 0.929f, 0.176f, 1.0f), true)
    };

    // Создание сетки
    grid = new Grid(20000.0f, 25);

    // Настройка шейдеров
    shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
    glUseProgram(shaderProgram);

    // Настройка проекции
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f), 
        800.0f/600.0f, 
        0.1f, 
        750000.0f
    );
    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "projection"),
        1, 
        GL_FALSE, 
        glm::value_ptr(projection)
    );
}

void Cleanup() {
    delete grid;
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
}

int main() {
    InitializeGLFW();
    InitializeGLEW();
    
    // Настройка OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, 800, 600);

    SetupScene();

    // Основной цикл
    while (!glfwWindowShouldClose(window) && running) {
        // Расчет времени
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Обновление
        camera.updateViewMatrix(shaderProgram);
        grid->Update(objects);

        // Физика
        if (!pause) {
            for (auto& obj : objects) {
                obj.UpdatePosition(false);
            }
        }

        // Отрисовка
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Отрисовка сетки
        GLint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
        glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 0.25f);
        glUniform1i(glGetUniformLocation(shaderProgram, "isGrid"), 1);
        grid->Draw(shaderProgram);

        // Отрисовка объектов
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniform1i(glGetUniformLocation(shaderProgram, "isGrid"), 0);
        
        for (auto& obj : objects) {
            glUniform4f(colorLoc, 
                      obj.GetColor().r, 
                      obj.GetColor().g, 
                      obj.GetColor().b, 
                      obj.GetColor().a);
            obj.Draw(shaderProgram);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Cleanup();
    return 0;
}