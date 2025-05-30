#include "Grid.hpp"      
#include "utils.hpp"     
#include <cmath>         
#include <algorithm>  

Grid::Grid(float size, int divs) : gridSize(size), divisions(divs) {
    float step = gridSize / divisions;
    float halfSize = gridSize / 2.0f;
    initialYPlane = -halfSize * 0.3f + 3 * step;

    generateInitialVertices();

}

Grid::~Grid() {
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
}

void Grid::generateInitialVertices() {
    vertices.clear();
    float step = gridSize / divisions;
    float halfSize = gridSize / 2.0f;

    // x-axis lines
    for (int zStep = 0; zStep <= divisions; ++zStep) {
        float z = -halfSize + zStep * step;
        for (int xStep = 0; xStep < divisions; ++xStep) {
            float xStart = -halfSize + xStep * step;
            float xEnd = xStart + step;
            vertices.insert(vertices.end(), { xStart, initialYPlane, z });
            vertices.insert(vertices.end(), { xEnd,   initialYPlane, z });
        }
    }
    // z-axis lines
    for (int xStep = 0; xStep <= divisions; ++xStep) {
        float x = -halfSize + xStep * step;
        for (int zStep = 0; zStep < divisions; ++zStep) {
            float zStart = -halfSize + zStep * step;
            float zEnd = zStart + step;
            vertices.insert(vertices.end(), { x, initialYPlane, zStart });
            vertices.insert(vertices.end(), { x, initialYPlane, zEnd });
        }
    }
    vertexCount = vertices.size(); 
}

void Grid::setupOpenGLResources() {
    if (!vertices.empty() && VAO == 0) { 
        Utils::createVBOVAO(VAO, VBO, vertices.data(), vertexCount, GL_DYNAMIC_DRAW);
    }
}

void Grid::updateAndWarp(const std::vector<Object>& objects) {
    if (vertices.empty() || VAO == 0) return;

    float totalMass = 0.0f;
    float comY = 0.0f;
    for (const auto& obj : objects) {
        if (obj.Initializing) continue; 
        comY += obj.mass * obj.position.y;
        totalMass += obj.mass;
    }
    if (totalMass > 0) comY /= totalMass;
    else comY = initialYPlane; 

    float verticalShiftFactor = comY - initialYPlane;

    for (size_t i = 0; i < vertices.size(); i += 3) { 

        glm::vec3 vertexBasePos(vertices[i], initialYPlane, vertices[i + 2]); 
        float totalDisplacementY = 0.0f;

        for (const auto& obj : objects) {
            if (obj.mass <= 0 || obj.radius <=0) continue; 

            glm::vec3 toObject = obj.position - vertexBasePos;
            glm::vec3 toObjectXZ = glm::vec3(toObject.x, 0.0f, toObject.z);
            float distanceXZ = glm::length(toObjectXZ);
            if (distanceXZ < 1.0f) distanceXZ = 1.0f; 

            float distanceXZ_m = distanceXZ * 1000.0f; 
            float rs = (2.0f * static_cast<float>(Constants::G) * obj.mass) / (Constants::C * Constants::C); 

            if (distanceXZ_m > rs) { 
                float warpFactor = (obj.mass / Constants::DEFAULT_INIT_MASS) * obj.radius * 100000.0f;

                totalDisplacementY -= warpFactor * (rs / distanceXZ_m);
            } 
        }

        vertices[i + 1] = initialYPlane + totalDisplacementY - std::abs(verticalShiftFactor * 0.1f);
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Grid::draw(Shader& shader) {
    if (VAO == 0 || vertexCount == 0) return;
    shader.use();
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    shader.setVec4("objectColor", glm::vec4(1.0f, 1.0f, 1.0f, 0.25f)); 
    shader.setBool("isGrid", true);
    shader.setBool("GLOW", false); 

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, vertexCount / 3); 
    glBindVertexArray(0);
}