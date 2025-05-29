#ifndef UTILS_HPP
#define UTILS_HPP

#include <glm/glm.hpp>
#include <GL/glew.h>

namespace Utils {
    glm::vec3 sphericalToCartesian(float r, float theta, float phi);
    void createVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount, GLenum usage = GL_STATIC_DRAW);
}

#endif 