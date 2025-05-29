#include "Shader.hpp"

class Shader {
    public:
        GLuint ID;
    
        Shader(const char* vertexSource, const char* fragmentSource) {
            GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
            GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
            linkProgram(vertexShader, fragmentShader);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
        }
    
        ~Shader() {
            glDeleteProgram(ID);
        }
    
        void use() const {
            glUseProgram(ID);
        }
    
        void setBool(const std::string& name, bool value) const {
            glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
        }
        void setInt(const std::string& name, int value) const {
            glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
        }
        void setFloat(const std::string& name, float value) const {
            glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
        }
        void setVec3(const std::string& name, const glm::vec3& value) const {
            glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
        }
        void setVec4(const std::string& name, const glm::vec4& value) const {
            glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
        }
        void setMat4(const std::string& name, const glm::mat4& mat) const {
            glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }
    
    private:
        GLuint compileShader(GLenum type, const char* source) {
            GLuint shader = glCreateShader(type);
            glShaderSource(shader, 1, &source, nullptr);
            glCompileShader(shader);
            checkCompileErrors(shader, type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");
            return shader;
        }
    
        void linkProgram(GLuint vertexShader, GLuint fragmentShader) {
            ID = glCreateProgram();
            glAttachShader(ID, vertexShader);
            glAttachShader(ID, fragmentShader);
            glLinkProgram(ID);
            checkCompileErrors(ID, "PROGRAM");
        }
    
        void checkCompileErrors(GLuint shader, std::string type) {
            GLint success;
            GLchar infoLog[1024];
            if (type != "PROGRAM") {
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
                if (!success) {
                    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                    std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
                }
            } else {
                glGetProgramiv(shader, GL_LINK_STATUS, &success);
                if (!success) {
                    glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                    std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
                }
            }
        }
    };
    