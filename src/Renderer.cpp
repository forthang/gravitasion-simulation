#include "Renderer.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

Renderer::Renderer(int screenWidth, int screenHeight)
    : width(screenWidth), height(screenHeight), shader(nullptr), vao(0), vbo(0) {}

Renderer::~Renderer() {
    delete shader;
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

bool Renderer::init() {
    shader = new Shader("shaders/vertex.glsl", "shaders/fragment.glsl");
    setupGraphics();
    return true;
}

void Renderer::setupGraphics() {
    float circleVerts[60 * 2]; // 60 points on circle
    const int segments = 60;
    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * 3.1415926f * i / segments;
        circleVerts[i * 2] = cos(angle);     // x
        circleVerts[i * 2 + 1] = sin(angle); // y
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circleVerts), circleVerts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

void Renderer::render(const std::vector<Body>& bodies, const Camera& camera) {
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader->use();

    // Передача матриц камеры
    shader->setMat4("uView", camera.getViewMatrix());
    shader->setMat4("uProj", camera.getProjectionMatrix());

    glBindVertexArray(vao);

    for (const auto& body : bodies) {
        glm::vec2 pos = body.getPosition();
        float mass = body.getMass();

        shader->setVec2("uPosition", pos.x, pos.y);
        shader->setFloat("uScale", std::max(2.0f, static_cast<float>(sqrt(mass))));
        shader->setVec3("uColor", 1.0f, 0.7f, 0.2f);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 60);
    }

    glBindVertexArray(0);
}
