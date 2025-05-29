#include "GravitySimulation.hpp" 
#include "constants.hpp"         

#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>      
#include <iostream>                    
#include <cmath>                        
#include <algorithm>                   

GravitySimulation::GravitySimulation(int width, int height, const char* title)
    : camera(glm::vec3(0.0f, 1000.0f, 5000.0f)), 
      grid(20000.0f, 25)
{
    if (!initGLFW(width, height, title)) {
        running = false; 
        return;
    }
    if (!initGLEW()) {
        running = false; 
        glfwTerminate(); 
        return;
    }
    initOpenGLOptions();
    
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
            // Assuming aPos are relative to object center, so aPos can be used as normal for a sphere
            vec3 normal = normalize(aPos); 
            // Simplistic light direction: from object surface point towards world origin (or camera for more complex)
            vec3 dirToLight = normalize(-worldPos); // Light at origin
            lightIntensity = max(dot(normal, dirToLight), 0.15); // Ambient term 0.15
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
                FragColor = vec4(objectColor.rgb * 2.0, objectColor.a); // Simple glow: brighten and keep alpha
            } else {
                float fade = smoothstep(0.0, 1.0, lightIntensity); // Smoothstep for softer lighting
                FragColor = vec4(objectColor.rgb * fade, objectColor.a);
            }
        }
    )glsl";
    mainShader = new Shader(vertexShaderSource, fragmentShaderSource);

    objects.emplace_back(Object(glm::vec3(-5000, 650, -350), glm::vec3(0, 0, -500), 5.97219e22f, 5515, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)));
    objects.emplace_back(Object(glm::vec3(5000, 650, -350), glm::vec3(0, 0, 500), 5.97219e22f, 5515, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)));
    objects.emplace_back(Object(glm::vec3(0, 0, -350), glm::vec3(0, 0, 0), 1.989e25f, 5515, glm::vec4(1.0f, 0.929f, 0.176f, 1.0f), true)); 
    
    for(auto& obj : objects) { 
        obj.Initializing = false;
        obj.Launched = true;
    }
    firstMouse = true; 
}

GravitySimulation::~GravitySimulation() {
    delete mainShader; 

    if (window) { 
        glfwTerminate();
    }
}

bool GravitySimulation::initGLFW(int width, int height, const char* title) {
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

        return false;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
    return true;
}

bool GravitySimulation::initGLEW() {
    glewExperimental = GL_TRUE; 
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }
    return true;
}

void GravitySimulation::initOpenGLOptions() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
}

void GravitySimulation::setupCallbacks() {
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
    });
}

void GravitySimulation::run() {
    glm::mat4 projection;

    while (running && !glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput();
        update(); 

        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        if (fbHeight > 0) { 
             projection = glm::perspective(glm::radians(camera.Zoom), (float)fbWidth / (float)fbHeight, 0.1f, 750000.0f);
        } else { 
             projection = glm::perspective(glm::radians(camera.Zoom), 1.0f, 0.1f, 750000.0f);
        }

        render(projection);

        glfwPollEvents();
    }
}

void GravitySimulation::processInput() {

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
         running = false; 
         glfwSetWindowShouldClose(window, true); 
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(GLFW_KEY_W, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(GLFW_KEY_S, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(GLFW_KEY_A, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(GLFW_KEY_D, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.ProcessKeyboard(GLFW_KEY_SPACE, deltaTime);
    
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && !isCreatingObject) {
        camera.ProcessKeyboard(GLFW_KEY_LEFT_SHIFT, deltaTime);
    }

    if (isCreatingObject && !objects.empty()) {
        Object& newObj = objects.back();
        bool shiftIsForVertical = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || 
                                   glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

        float moveSpeed = (newObj.radius > 0.001f ? newObj.radius : 1.0f) * 20.0f * deltaTime; 

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

void GravitySimulation::update() {
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
                float distance_visual = glm::length(diff);

                if (distance_visual > 0.001f) { 
                    glm::vec3 direction = glm::normalize(diff);
 
                    float dist_m = distance_visual * 1000.0f; 
                    
                    double G_force_mag = (Constants::G * static_cast<double>(objects[i].mass) * static_cast<double>(objects[j].mass)) / 
                                         (static_cast<double>(dist_m) * dist_m);

                    float acc_mag = static_cast<float>(G_force_mag / static_cast<double>(objects[i].mass));
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

void GravitySimulation::render(const glm::mat4& projection) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!mainShader) return; 

    mainShader->use();
    mainShader->setMat4("projection", projection);
    mainShader->setMat4("view", camera.GetViewMatrix());

    grid.draw(*mainShader);

    for (auto& obj : objects) {

        obj.draw(*mainShader);
    }

    glfwSwapBuffers(window);
}


void GravitySimulation::keyCallback(int key, int scancode, int action, int mods) {

    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        paused = !paused;
        std::cout << "Simulation " << (paused ? "PAUSED" : "RESUMED") << std::endl;
    }
}

void GravitySimulation::mouseCallback(double xpos, double ypos) {
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

void GravitySimulation::mouseButtonCallback(int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS && !isCreatingObject) {
            glm::vec3 startPos = camera.Position + camera.Front * 500.0f; 
            objects.emplace_back(Object(startPos, glm::vec3(0.0f), Constants::DEFAULT_INIT_MASS));
            objects.back().Initializing = true;
            objects.back().Launched = false; 
            isCreatingObject = true;
            std::cout << "Started creating object. Initial Mass: " << objects.back().mass 
                      << ", Visual Radius: " << objects.back().radius << std::endl;
        } else if (action == GLFW_RELEASE && isCreatingObject && !objects.empty()) {
            objects.back().Initializing = false;
            objects.back().Launched = true; 
            isCreatingObject = false;
            std::cout << "Launched object. Final Mass: " << objects.back().mass 
                      << ", Visual Radius: " << objects.back().radius << std::endl;
        }
    }
}

void GravitySimulation::scrollCallback(double xoffset, double yoffset) {
    if (deltaTime > 0.0f) { 
        camera.ProcessMouseScroll(static_cast<float>(yoffset), deltaTime);
    }
}