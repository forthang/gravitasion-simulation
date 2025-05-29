// main.cpp
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath> // For std::pow, std::sqrt, std::sin, std::cos
#include <algorithm> // For std::max

// --- Constants ---
namespace Constants {
    const double G = 6.6743e-11; // m^3 kg^-1 s^-2
    const float C = 299792458.0f;
    const float DEFAULT_INIT_MASS = static_cast<float>(pow(10, 22));
    const float DEFAULT_SIZE_RATIO = 30000.0f;
    const float PI = 3.14159265359f;
}

// --- Utility Functions (can be part of a MathUtils class or namespace) ---
namespace Utils {
    glm::vec3 sphericalToCartesian(float r, float theta, float phi) {
        float x = r * std::sin(theta) * std::cos(phi);
        float y = r * std::cos(theta);
        float z = r * std::sin(theta) * std::sin(phi);
        return glm::vec3(x, y, z);
    }

    void createVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount, GLenum usage = GL_STATIC_DRAW) {
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

// --- Shader Class ---
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

// --- Camera Class ---
class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom; // For FOV

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f, float pitch = 0.0f)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(10000.0f), MouseSensitivity(0.1f), Zoom(45.0f) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(int key, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (key == GLFW_KEY_W) Position += Front * velocity;
        if (key == GLFW_KEY_S) Position -= Front * velocity;
        if (key == GLFW_KEY_A) Position -= Right * velocity;
        if (key == GLFW_KEY_D) Position += Right * velocity;
        if (key == GLFW_KEY_SPACE) Position += WorldUp * velocity; // Use WorldUp for consistent up/down
        if (key == GLFW_KEY_LEFT_SHIFT) Position -= WorldUp * velocity;
    }

    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        if (constrainPitch) {
            if (Pitch > 89.0f) Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;
        }
        updateCameraVectors();
    }

    void ProcessMouseScroll(float yoffset, float deltaTime) {
         float scrollSpeed = MovementSpeed * 5.0f * deltaTime; // Make scroll speed proportional
        if (yoffset > 0) {
            Position += Front * scrollSpeed;
        } else if (yoffset < 0) {
            Position -= Front * scrollSpeed;
        }
    }


private:
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};


// --- Object Class (Celestial Body) ---
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
    float sizeRatio; // To scale visual radius from physical radius

    bool glow;

    Object(glm::vec3 initPosition, glm::vec3 initVelocity, float m,
           float d = 3344.0f, glm::vec4 c = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
           bool g = false, float sr = Constants::DEFAULT_SIZE_RATIO)
        : position(initPosition), velocity(initVelocity), mass(m), density(d), color(c), glow(g), sizeRatio(sr) {
        updateRadius();
        generateSphereVertices();
    }

    ~Object() {
        if (VAO != 0) glDeleteVertexArrays(1, &VAO);
        if (VBO != 0) glDeleteBuffers(1, &VBO);
    }

    Object(const Object& other)
        : position(other.position), velocity(other.velocity), vertexCount(other.vertexCount),
          color(other.color), Initializing(other.Initializing), Launched(other.Launched),
          mass(other.mass), density(other.density), radius(other.radius), sizeRatio(other.sizeRatio), glow(other.glow)
    {
        VAO = 0; VBO = 0;
        if (vertexCount > 0) {
            generateSphereVertices();
        }
    }

    Object& operator=(const Object& other) {
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

    Object(Object&& other) noexcept
        : VAO(other.VAO), VBO(other.VBO), position(std::move(other.position)), velocity(std::move(other.velocity)),
          vertexCount(other.vertexCount), color(std::move(other.color)), Initializing(other.Initializing),
          Launched(other.Launched), mass(other.mass), density(other.density), radius(other.radius),
          sizeRatio(other.sizeRatio), glow(other.glow)
    {
        other.VAO = 0;
        other.VBO = 0;
        other.vertexCount = 0;
    }

    Object& operator=(Object&& other) noexcept {
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


    void updateRadius() {
        if (this->density <= 0) this->density = 3344.0f; // Prevent division by zero or negative radius
        if (this->mass <=0) this->mass = Constants::DEFAULT_INIT_MASS; // Prevent issues with zero mass
        this->radius = std::pow(((3.0f * this->mass / this->density) / (4.0f * Constants::PI)), (1.0f / 3.0f)) / sizeRatio;
        if (this->radius <= 0) this->radius = 0.1f / sizeRatio; // Ensure positive radius
    }

    void generateSphereVertices() {
        if (VAO != 0) glDeleteVertexArrays(1, &VAO);
        if (VBO != 0) glDeleteBuffers(1, &VBO);

        std::vector<float> vertices_local; // Use a local vector
        int stacks = 10;
        int sectors = 10;
        float current_radius = this->radius > 0.0f ? this->radius : 0.001f; // Ensure radius is positive for generation

        for (float i = 0.0f; i <= stacks; ++i) {
            float theta1 = (i / stacks) * Constants::PI;
            float theta2 = (i + 1) / stacks * Constants::PI;
            for (float j = 0.0f; j < sectors; ++j) {
                float phi1 = j / sectors * 2 * Constants::PI;
                float phi2 = (j + 1) / sectors * 2 * Constants::PI;
                glm::vec3 v1 = Utils::sphericalToCartesian(current_radius, theta1, phi1);
                glm::vec3 v2 = Utils::sphericalToCartesian(current_radius, theta1, phi2);
                glm::vec3 v3 = Utils::sphericalToCartesian(current_radius, theta2, phi1);
                glm::vec3 v4 = Utils::sphericalToCartesian(current_radius, theta2, phi2);

                vertices_local.insert(vertices_local.end(), { v1.x, v1.y, v1.z });
                vertices_local.insert(vertices_local.end(), { v2.x, v2.y, v2.z });
                vertices_local.insert(vertices_local.end(), { v3.x, v3.y, v3.z });
                vertices_local.insert(vertices_local.end(), { v2.x, v2.y, v2.z });
                vertices_local.insert(vertices_local.end(), { v4.x, v4.y, v4.z });
                vertices_local.insert(vertices_local.end(), { v3.x, v3.y, v3.z });
            }
        }
        vertexCount = vertices_local.size(); // Update member vertexCount
        if (!vertices_local.empty()) {
             Utils::createVBOVAO(VAO, VBO, vertices_local.data(), vertexCount, GL_DYNAMIC_DRAW);
        }
    }

    void updatePhysics(float timeStepRatio = 94.0f) {
        this->position += this->velocity / timeStepRatio;
    }

    void accelerate(const glm::vec3& acc, float timeStepRatio = 96.0f) {
        this->velocity += acc / timeStepRatio;
    }

    float checkCollision(const Object& other) {
        glm::vec3 diff = other.position - this->position;
        float distance = glm::length(diff);
        if (other.radius + this->radius > distance) {
            return -0.2f; // Collision factor
        }
        return 1.0f; // No collision
    }

    void draw(Shader& shader) {
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
};

// --- Grid Class ---
class Grid {
public:
    GLuint VAO = 0, VBO = 0;
    size_t vertexCount = 0;
    std::vector<float> vertices;
    float gridSize;
    int divisions;
    float initialYPlane;


    Grid(float size = 20000.0f, int divs = 25) : gridSize(size), divisions(divs) {
        float step = gridSize / divisions;
        float halfSize = gridSize / 2.0f;
        initialYPlane = -halfSize * 0.3f + 3 * step; // Store initial Y plane

        generateInitialVertices();
        // OpenGL resources will be set up later via setupOpenGLResources()
    }

    ~Grid() {
        if (VAO != 0) glDeleteVertexArrays(1, &VAO);
        if (VBO != 0) glDeleteBuffers(1, &VBO);
    }

    Grid(const Grid&) = delete;
    Grid& operator=(const Grid&) = delete;
    Grid(Grid&&) = delete;
    Grid& operator=(Grid&&) = delete;


    void generateInitialVertices() {
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

    void setupOpenGLResources() {
        if (!vertices.empty() && VAO == 0) { // Check if not already initialized
            Utils::createVBOVAO(VAO, VBO, vertices.data(), vertexCount, GL_DYNAMIC_DRAW);
        }
    }


    void updateAndWarp(const std::vector<Object>& objects) {
        if (vertices.empty() || VAO == 0) return;

        float totalMass = 0.0f;
        float comY = 0.0f;
        for (const auto& obj : objects) {
            if (obj.Initializing) continue;
            comY += obj.mass * obj.position.y;
            totalMass += obj.mass;
        }
        if (totalMass > 0) comY /= totalMass;
        else comY = initialYPlane; // Default to initial plane if no mass

        float originalGridY = initialYPlane;
        float verticalShiftFactor = comY - originalGridY;


        for (size_t i = 0; i < vertices.size(); i += 3) { // Iterate over x,y,z components
            // vertices[i] = X, vertices[i+1] = Y, vertices[i+2] = Z
            // The X and Z coordinates of grid vertices do not change.
            // Only Y (vertices[i+1]) is warped.

            glm::vec3 vertexPos(vertices[i], initialYPlane, vertices[i + 2]); // Use initialY for warping calculation base
            float totalDisplacementY = 0.0f;

            for (const auto& obj : objects) {
                if (obj.mass <= 0 || obj.radius <=0) continue;

                glm::vec3 toObject = obj.position - vertexPos;
                glm::vec3 toObjectXZ = glm::vec3(toObject.x, 0.0f, toObject.z);
                float distanceXZ = glm::length(toObjectXZ);
                if (distanceXZ < 1.0f) distanceXZ = 1.0f;

                float distanceXZ_m = distanceXZ * 1000.0f;
                float rs = (2.0f * static_cast<float>(Constants::G) * obj.mass) / (Constants::C * Constants::C);

                if (distanceXZ_m > rs) {
                    float warpFactor = (obj.mass / Constants::DEFAULT_INIT_MASS) * obj.radius * 100000.0f;
                    totalDisplacementY -= warpFactor * (rs / distanceXZ_m);
                } // какашки
            }
            vertices[i + 1] = initialYPlane + totalDisplacementY - std::abs(verticalShiftFactor * 0.1f);
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data()); // Use glBufferSubData for updates
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }


    void draw(Shader& shader) {
        if (VAO == 0 || vertexCount == 0) return;
        shader.use();
        glm::mat4 model = glm::mat4(1.0f);
        shader.setMat4("model", model);
        shader.setVec4("objectColor", glm::vec4(1.0f, 1.0f, 1.0f, 0.25f));
        shader.setBool("isGrid", true);
        shader.setBool("GLOW", false);

        glBindVertexArray(VAO);
        glPointSize(2.0f);
        glDrawArrays(GL_LINES, 0, vertexCount / 3);
        glBindVertexArray(0);
    }
};


// --- GravitySimulation (Application) Class ---
class GravitySimulation {
public:
    GLFWwindow* window = nullptr;
    Shader* mainShader = nullptr;
    Camera camera;
    std::vector<Object> objects;
    Grid grid;

    bool running = true;
    bool paused = true;
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    float lastMouseX = 400.0f, lastMouseY = 300.0f;
    bool firstMouse = true;

    bool isCreatingObject = false;

    GravitySimulation(int width, int height, const char* title)
        : camera(glm::vec3(0.0f, 1000.0f, 5000.0f)), grid(20000.0f, 25) {
        if (!initGLFW(width, height, title)) {
            running = false;
            return;
        }
        if (!initGLEW()) {
            running = false;
            return;
        }
        initOpenGLOptions();
        
        // Setup Grid's OpenGL resources after GLFW and GLEW are initialized
        grid.setupOpenGLResources();

        setupCallbacks();

        const char* vertexShaderSource = R"glsl(
            #version 330 core
            layout(location=0) in vec3 aPos;
            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;
            out float lightIntensity;
            void main() {
                gl_Position = projection * view * model * vec4(aPos, 1.0);
                vec3 worldPos = (model * vec4(aPos, 1.0)).xyz;
                vec3 normal = normalize(aPos);
                vec3 dirToCenter = normalize(-worldPos);
                lightIntensity = max(dot(normal, dirToCenter), 0.15);
            }
        )glsl";

        const char* fragmentShaderSource = R"glsl(
            #version 330 core
            in float lightIntensity;
            out vec4 FragColor;
            uniform vec4 objectColor;
            uniform bool isGrid;
            uniform bool GLOW;
            void main() {
                if (isGrid) {
                    FragColor = objectColor;
                } else if(GLOW){
                    FragColor = vec4(objectColor.rgb * 2.0, objectColor.a);
                } else {
                    float fade = smoothstep(0.0, 1.0, lightIntensity);
                    FragColor = vec4(objectColor.rgb * fade, objectColor.a);
                }
            }
        )glsl";
        mainShader = new Shader(vertexShaderSource, fragmentShaderSource);

        objects.emplace_back(Object(glm::vec3(-5000, 650, -350), glm::vec3(0, 0, 1500), 5.97219e22f, 5515, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)));
        objects.emplace_back(Object(glm::vec3(5000, 650, -350), glm::vec3(0, 0, -1500), 5.97219e22f, 5515, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)));
        objects.emplace_back(Object(glm::vec3(0, 0, -350), glm::vec3(0, 0, 0), 1.989e25f, 5515, glm::vec4(1.0f, 0.929f, 0.176f, 1.0f), true));

        for(auto& obj : objects) { // Ensure initial objects are not in "initializing" state
            obj.Initializing = false;
            obj.Launched = true;
        }
        firstMouse = true;
    }

    ~GravitySimulation() {
        delete mainShader;
        glfwTerminate();
    }

    void run() {
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)fbWidth / (float)fbHeight, 0.1f, 750000.0f);

        while (running && !glfwWindowShouldClose(window)) {
            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            processInput();
            update();

            // Update projection matrix if window resized (handled by callback for viewport, but projection needs explicit update here or in callback)
            glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
            if (fbHeight > 0) { // avoid division by zero
                 projection = glm::perspective(glm::radians(camera.Zoom), (float)fbWidth / (float)fbHeight, 0.1f, 750000.0f);
            }

            render(projection);

            glfwPollEvents();
        }
    }

private:
    bool initGLFW(int width, int height, const char* title) {
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        #endif


        window = glfwCreateWindow(width, height, title, NULL, NULL);
        if (!window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(window);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        return true;
    }

    bool initGLEW() {
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW" << std::endl;
            return false;
        }
        return true;
    }

    void initOpenGLOptions() {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        int width, height;
        glfwGetFramebufferSize(window, &width, &height); // Get actual framebuffer size
        glViewport(0, 0, width, height);
    }

    void setupCallbacks() {
        glfwSetWindowUserPointer(window, this);

        glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int sc, int act, int mods) {
            static_cast<GravitySimulation*>(glfwGetWindowUserPointer(w))->keyCallback(key, sc, act, mods);
        });
        glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y) {
            static_cast<GravitySimulation*>(glfwGetWindowUserPointer(w))->mouseCallback(x, y);
        });
        glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int b, int a, int m) {
            static_cast<GravitySimulation*>(glfwGetWindowUserPointer(w))->mouseButtonCallback(b, a, m);
        });
        glfwSetScrollCallback(window, [](GLFWwindow* w, double xoff, double yoff) {
            static_cast<GravitySimulation*>(glfwGetWindowUserPointer(w))->scrollCallback(xoff, yoff);
        });
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int width, int height) {
            glViewport(0, 0, width, height);
            // GravitySimulation* sim = static_cast<GravitySimulation*>(glfwGetWindowUserPointer(w));
            // if (sim && height > 0) {
            //    sim->projection = glm::perspective(glm::radians(sim->camera.Zoom), (float)width / (float)height, 0.1f, 750000.0f);
            // } // Projection matrix updated in run loop to ensure it's always current
        });
    }

    void processInput() {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(GLFW_KEY_W, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(GLFW_KEY_S, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(GLFW_KEY_A, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(GLFW_KEY_D, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.ProcessKeyboard(GLFW_KEY_SPACE, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && !isCreatingObject) camera.ProcessKeyboard(GLFW_KEY_LEFT_SHIFT, deltaTime); // Ensure shift for camera doesn't conflict with obj creation


        if (isCreatingObject && !objects.empty()) {
            Object& newObj = objects.back();
            bool shiftIsForVertical = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

            float moveSpeed = (newObj.radius > 0 ? newObj.radius : 1.0f) * 20.0f * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
                newObj.position += (shiftIsForVertical ? camera.WorldUp : camera.Front) * moveSpeed;
            }
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
                newObj.position -= (shiftIsForVertical ? camera.WorldUp : camera.Front) * moveSpeed;
            }
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
                newObj.position -= camera.Right * moveSpeed;
            }
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
                newObj.position += camera.Right * moveSpeed;
            }
        }
    }

    void update() {
        if (isCreatingObject && !objects.empty()) {
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
                Object& newObj = objects.back();
                newObj.mass *= (1.0f + 1.0f * deltaTime);
                newObj.updateRadius();
                newObj.generateSphereVertices();
            }
        }

        if (!paused) {
            for (size_t i = 0; i < objects.size(); ++i) {
                if (objects[i].Initializing || !objects[i].Launched) continue;

                for (size_t j = 0; j < objects.size(); ++j) {
                    if (i == j || objects[j].Initializing || !objects[j].Launched) continue;

                    glm::vec3 diff = objects[j].position - objects[i].position;
                    float distance = glm::length(diff);

                    if (distance > 0.001f) {
                        glm::vec3 direction = glm::normalize(diff);
                        float dist_m = distance * 1000.0f;
                        double G_force_mag = (Constants::G * objects[i].mass * objects[j].mass) / (static_cast<double>(dist_m) * dist_m);

                        float acc_mag = static_cast<float>(G_force_mag / objects[i].mass);
                        glm::vec3 acceleration = direction * acc_mag;
                        objects[i].accelerate(acceleration);
                    }
                    float collisionFactor = objects[i].checkCollision(objects[j]);
                    if (collisionFactor < 1.0f) {
                        objects[i].velocity *= collisionFactor;
                    }
                }
            }

            for (auto& obj : objects) {
                if (!obj.Initializing && obj.Launched) {
                    obj.updatePhysics();
                }
            }
        }
         grid.updateAndWarp(objects);
    }

    void render(const glm::mat4& projection) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mainShader->use();
        mainShader->setMat4("projection", projection);
        mainShader->setMat4("view", camera.GetViewMatrix());

        grid.draw(*mainShader);

        for (auto& obj : objects) {
            obj.draw(*mainShader);
        }

        glfwSwapBuffers(window);
    }


    void keyCallback(int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            running = false;
            glfwSetWindowShouldClose(window, true);
        }
        if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
             running = false;
             glfwSetWindowShouldClose(window, true);
        }

        if (key == GLFW_KEY_P && action == GLFW_PRESS) {
            paused = !paused;
        }
    }

    void mouseCallback(double xpos, double ypos) {
        if (firstMouse) {
            lastMouseX = static_cast<float>(xpos);
            lastMouseY = static_cast<float>(ypos);
            firstMouse = false;
        }

        float xoffset = static_cast<float>(xpos) - lastMouseX;
        float yoffset = lastMouseY - static_cast<float>(ypos);
        lastMouseX = static_cast<float>(xpos);
        lastMouseY = static_cast<float>(ypos);

        camera.ProcessMouseMovement(xoffset, yoffset);
    }

    void mouseButtonCallback(int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS && !isCreatingObject) {
                glm::vec3 startPos = camera.Position + camera.Front * 500.0f;
                objects.emplace_back(Object(startPos, glm::vec3(0.0f), Constants::DEFAULT_INIT_MASS));
                objects.back().Initializing = true;
                objects.back().Launched = false; // Not launched yet
                isCreatingObject = true;
                 std::cout << "Started creating object. Initial Mass: " << objects.back().mass << std::endl;
            } else if (action == GLFW_RELEASE && isCreatingObject && !objects.empty()) {
                objects.back().Initializing = false;
                objects.back().Launched = true; // Now it's launched
                isCreatingObject = false;
                 std::cout << "Launched object. Final Mass: " << objects.back().mass << std::endl;
            }
        }
    }

    void scrollCallback(double xoffset, double yoffset) {
        if (deltaTime > 0) { // Ensure deltaTime is valid
            camera.ProcessMouseScroll(static_cast<float>(yoffset), deltaTime);
        }
    }
};


// --- Main Function ---
int main() {
    GravitySimulation sim(1280, 720, "Gravity Simulation OOP"); // Increased default window size
    if (sim.running) {
        sim.run();
    }
    return 0;
}