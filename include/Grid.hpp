#ifndef GRID_HPP
#define GRID_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include "Shader.hpp"  
#include "Object.hpp"    
#include "constants.hpp"

class Grid {
public:
    GLuint VAO = 0, VBO = 0;
    size_t vertexCount = 0;
    float gridSize;
    int divisions;
    float initialYPlane;

    Grid(float size = 20000.0f, int divs = 25);
    ~Grid();

    Grid(const Grid&) = delete;
    Grid& operator=(const Grid&) = delete;
    Grid(Grid&&) = delete;
    Grid& operator=(Grid&&) = delete;

    void setupOpenGLResources(); 
    void updateAndWarp(const std::vector<Object>& objects);
    void draw(Shader& shader);

private:
    std::vector<float> vertices; 
    void generateInitialVertices();
};

#endif