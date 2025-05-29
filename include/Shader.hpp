#ifndef SHADER_HPP
#define SHADER_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
public:
    GLuint ID;

    Shader(const char* vertexSource, const char* fragmentSource);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&&) = delete;
    Shader& operator=(Shader&&) = delete;

    void use() const;
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

private:
    GLuint compileShader(GLenum type, const char* source);
    void linkProgram(GLuint vertexShader, GLuint fragmentShader);
    void checkCompileErrors(GLuint shader, std::string type);
};

#endif