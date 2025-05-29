#include "Camera.hpp" 
#include <cmath>     

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(10000.0f), MouseSensitivity(0.1f), Zoom(45.0f) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(int key, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (key == GLFW_KEY_W) Position += Front * velocity;
    if (key == GLFW_KEY_S) Position -= Front * velocity;
    if (key == GLFW_KEY_A) Position -= Right * velocity;
    if (key == GLFW_KEY_D) Position += Right * velocity;
    if (key == GLFW_KEY_SPACE) Position += WorldUp * velocity;
    if (key == GLFW_KEY_LEFT_SHIFT) Position -= WorldUp * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
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

void Camera::ProcessMouseScroll(float yoffset, float deltaTime) {
    float scrollSpeed = MovementSpeed * 5.0f * deltaTime;
    if (yoffset > 0) {
        Position += Front * scrollSpeed;
    } else if (yoffset < 0) {
        Position -= Front * scrollSpeed;
    }
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = std::cos(glm::radians(Yaw)) * std::cos(glm::radians(Pitch));
    front.y = std::sin(glm::radians(Pitch));
    front.z = std::sin(glm::radians(Yaw)) * std::cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}