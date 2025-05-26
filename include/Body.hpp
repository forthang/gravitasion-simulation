#pragma once
#include <glm/glm.hpp>

class Body {
public:
    Body(glm::vec2 position, glm::vec2 velocity, float mass);

    void applyForce(const glm::vec2& force);
    void update(float dt);

    const glm::vec2& getPosition() const;
    const glm::vec2& getVelocity() const;
    float getMass() const;

private:
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec2 accumulatedForce;
    float mass;
};
