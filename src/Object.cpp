#include "Object.hpp"    
#include "utils.hpp"    
#include <cmath>        
#include <algorithm>     
#include <glm/gtc/matrix_transform.hpp> 
#include <utility>      

Object::Object(glm::vec3 initPosition, glm::vec3 initVelocity, float m,
               float d, glm::vec4 c, bool g, float sr)
    : position(initPosition), velocity(initVelocity), mass(m), density(d), color(c), glow(g), sizeRatio(sr) {
    updateRadius();
    generateSphereVertices();
}

Object::~Object() {
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
}

Object::Object(const Object& other)
    : position(other.position), velocity(other.velocity), vertexCount(other.vertexCount),
      color(other.color), Initializing(other.Initializing), Launched(other.Launched),
      mass(other.mass), density(other.density), radius(other.radius), sizeRatio(other.sizeRatio), glow(other.glow)
{
    VAO = 0; VBO = 0; 
    if (vertexCount > 0) {
        generateSphereVertices(); 
    }
}

Object& Object::operator=(const Object& other) {
    if (this == &other) return *this;

    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    VAO = 0; VBO = 0;

    position = other.position;
    velocity = other.velocity;
    vertexCount = other.vertexCount;
    color = other.color;
    Initializing = other.Initializing;
    Launched = other.Launched;
    mass = other.mass;
    density = other.density;
    radius = other.radius;
    sizeRatio = other.sizeRatio;
    glow = other.glow;

    if (vertexCount > 0) {
         generateSphereVertices(); 
    }
    return *this;
}

Object::Object(Object&& other) noexcept
    : VAO(other.VAO), VBO(other.VBO), 
      position(std::move(other.position)), velocity(std::move(other.velocity)),
      vertexCount(other.vertexCount), color(std::move(other.color)), 
      Initializing(other.Initializing), Launched(other.Launched),
      mass(other.mass), density(other.density), radius(other.radius),
      sizeRatio(other.sizeRatio), glow(other.glow)
{

    other.VAO = 0;
    other.VBO = 0;
    other.vertexCount = 0; 
}

Object& Object::operator=(Object&& other) noexcept {
    if (this == &other) return *this;

    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);

    VAO = other.VAO;
    VBO = other.VBO;
    position = std::move(other.position);
    velocity = std::move(other.velocity);
    vertexCount = other.vertexCount;
    color = std::move(other.color);
    Initializing = other.Initializing;
    Launched = other.Launched;
    mass = other.mass;
    density = other.density;
    radius = other.radius;
    sizeRatio = other.sizeRatio;
    glow = other.glow;

    other.VAO = 0;
    other.VBO = 0;
    other.vertexCount = 0;
    return *this;
}

void Object::updateRadius() {
    if (this->density <= 0) this->density = 3344.0f; 
    if (this->mass <=0) this->mass = Constants::DEFAULT_INIT_MASS; 
    // Считаем физ радиус и потом визуал исправить
    float physical_radius = std::pow(((3.0f * this->mass / this->density) / (4.0f * Constants::PI)), (1.0f / 3.0f));
    this->radius = physical_radius / sizeRatio;
    
    if (this->radius <= 0) { 
        this->radius = (0.1f / sizeRatio); 
    }
}

void Object::generateSphereVertices() {

    if (VAO != 0) { glDeleteVertexArrays(1, &VAO); VAO = 0; }
    if (VBO != 0) { glDeleteBuffers(1, &VBO); VBO = 0; }

    std::vector<float> vertices_local; 
    int stacks = 10;
    int sectors = 10;

    float current_visual_radius = this->radius > 0.0f ? this->radius : 0.001f; 

    for (float i = 0.0f; i <= stacks; ++i) {
        float theta1 = (i / stacks) * Constants::PI;
        float theta2 = (i + 1) / stacks * Constants::PI;
        for (float j = 0.0f; j < sectors; ++j) {
            float phi1 = j / sectors * 2 * Constants::PI;
            float phi2 = (j + 1) / sectors * 2 * Constants::PI;
            glm::vec3 v1 = Utils::sphericalToCartesian(current_visual_radius, theta1, phi1);
            glm::vec3 v2 = Utils::sphericalToCartesian(current_visual_radius, theta1, phi2);
            glm::vec3 v3 = Utils::sphericalToCartesian(current_visual_radius, theta2, phi1);
            glm::vec3 v4 = Utils::sphericalToCartesian(current_visual_radius, theta2, phi2);

            vertices_local.insert(vertices_local.end(), { v1.x, v1.y, v1.z });
            vertices_local.insert(vertices_local.end(), { v2.x, v2.y, v2.z });
            vertices_local.insert(vertices_local.end(), { v3.x, v3.y, v3.z });
            vertices_local.insert(vertices_local.end(), { v2.x, v2.y, v2.z });
            vertices_local.insert(vertices_local.end(), { v4.x, v4.y, v4.z });
            vertices_local.insert(vertices_local.end(), { v3.x, v3.y, v3.z });
        }
    }
    vertexCount = vertices_local.size(); 
    if (!vertices_local.empty()) {
         Utils::createVBOVAO(VAO, VBO, vertices_local.data(), vertexCount, GL_DYNAMIC_DRAW);
    } else {
        vertexCount = 0; 
    }
}

void Object::updatePhysics(float timeStepRatio) {
    this->position += this->velocity / timeStepRatio;
}

void Object::accelerate(const glm::vec3& acc, float timeStepRatio) {
    this->velocity += acc / timeStepRatio;
}

float Object::checkCollision(const Object& other) {
    glm::vec3 diff = other.position - this->position;
    float distance = glm::length(diff);
    if (other.radius + this->radius > distance) {
        return -0.2f; 
    }
    return 1.0f; 
}

void Object::draw(Shader& shader) {
    if (VAO == 0 || vertexCount == 0) return;

    shader.use();
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    shader.setMat4("model", model);
    shader.setVec4("objectColor", color);
    shader.setBool("isGrid", false);
    shader.setBool("GLOW", glow);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount / 3); 
    glBindVertexArray(0);
}