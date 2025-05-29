#include "utils.hpp" 
#include <cmath>     

namespace Utils {
    glm::vec3 sphericalToCartesian(float r, float theta, float phi) {
        float x = r * std::sin(theta) * std::cos(phi);
        float y = r * std::cos(theta);
        float z = r * std::sin(theta) * std::sin(phi);
        return glm::vec3(x, y, z);
    }

    void createVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount, GLenum usage) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices, usage);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }
}