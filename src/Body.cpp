#include "Body.hpp"

Body::Body(glm::vec2 position, glm::vec2 velocity, float mass)
    : position(position), velocity(velocity), mass(mass), accumulatedForce(0.0f, 0.0f) {}

void Body::applyForce(const glm::vec2& force) {
    accumulatedForce += force;
}

void Body::update(float dt) {
    if (mass <= 0.0f) return;

    glm::vec2 acceleration = accumulatedForce / mass;
    velocity += acceleration * dt;
    position += velocity * dt;

    // Очистить силу после применения
    accumulatedForce = glm::vec2(0.0f, 0.0f);
}

const glm::vec2& Body::getPosition() const {
    return position;
}

const glm::vec2& Body::getVelocity() const {
    return velocity;
}

float Body::getMass() const {
    return mass;
}
