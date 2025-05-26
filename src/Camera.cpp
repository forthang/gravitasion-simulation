#include "Camera.hpp"

Camera::Camera(int screenWidth, int screenHeight)
    : position(0.0f, 0.0f), zoomLevel(1.0f),
      screenWidth(screenWidth), screenHeight(screenHeight) {}

void Camera::move(const glm::vec2& delta) {
    position += delta / zoomLevel; // компенсируем масштаб
}

void Camera::zoom(float factor) {
    zoomLevel *= factor;
    if (zoomLevel < 0.1f) zoomLevel = 0.1f;
    if (zoomLevel > 10.0f) zoomLevel = 10.0f;
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::translate(glm::mat4(1.0f), glm::vec3(-position, 0.0f));
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::ortho(
        0.0f, static_cast<float>(screenWidth) / zoomLevel,
        0.0f, static_cast<float>(screenHeight) / zoomLevel,
        -1.0f, 1.0f
    );
}

void Camera::setScreenSize(int width, int height) {
    screenWidth = width;
    screenHeight = height;
}

const glm::vec2& Camera::getPosition() const {
    return position;
}

float Camera::getZoom() const {
    return zoomLevel;
}
