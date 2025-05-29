#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include "Shader.hpp"   
#include "constants.hpp" 

class Object {
public:
    GLuint VAO = 0, VBO = 0;
    glm::vec3 position;
    glm::vec3 velocity;
    size_t vertexCount = 0;
    glm::vec4 color;

    bool Initializing = false;
    bool Launched = false;

    float mass;
    float density;
    float radius;    
    float sizeRatio; 

    bool glow;

    Object(glm::vec3 initPosition, glm::vec3 initVelocity, float m,
           float d = 3344.0f, glm::vec4 c = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
           bool g = false, float sr = Constants::DEFAULT_SIZE_RATIO);

    ~Object();

   
    Object(const Object& other);
    Object& operator=(const Object& other);
    Object(Object&& other) noexcept;
    Object& operator=(Object&& other) noexcept;

    void updateRadius();
    void generateSphereVertices();
    void updatePhysics(float timeStepRatio = 94.0f);
    void accelerate(const glm::vec3& acc, float timeStepRatio = 96.0f);
    float checkCollision(const Object& other);
    void draw(Shader& shader);
};

#endif 