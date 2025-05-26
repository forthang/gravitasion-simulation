#pragma once
#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    void use() const;
    void setVec2(const std::string &name, float x, float y) const;
    void setVec3(const std::string &name, float x, float y, float z) const;
    void setFloat(const std::string &name, float value) const;
    void setMat4(const std::string& name, const glm::mat4& matrix) const;

private:
    unsigned int id;
    void checkCompileErrors(unsigned int shader, const std::string& type);
};
