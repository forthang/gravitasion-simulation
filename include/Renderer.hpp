#pragma once
#include "Shader.hpp"
#include "Body.hpp"
#include "Camera.hpp"
#include <vector>

class Renderer {
public:
    Renderer(int screenWidth, int screenHeight);
    ~Renderer();

    bool init();
    void render(const std::vector<Body>& bodies, const Camera& camera); 

private:
    int width, height;
    Shader* shader;

    unsigned int vao, vbo;
    void setupGraphics();
};
