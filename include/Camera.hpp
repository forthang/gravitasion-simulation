#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera(int screenWidth, int screenHeight);

    void move(const glm::vec2& delta);
    void zoom(float factor);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

    void setScreenSize(int width, int height);
    const glm::vec2& getPosition() const;
    float getZoom() const;

private:
    glm::vec2 position;
    float zoomLevel;

    int screenWidth;
    int screenHeight;
};
