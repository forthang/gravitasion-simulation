#include "PhysicsEngine.hpp"
#include <glm/glm.hpp>
#include <cmath>

const float G = 100.0f; // Гравитационная постоянная (произвольно подобрана)

void PhysicsEngine::addBody(const Body& body) {
    bodies.push_back(body);
}

std::vector<Body>& PhysicsEngine::getBodies() {
    return bodies;
}

void PhysicsEngine::update(float dt) {
    computeForces();

    for (auto& body : bodies) {
        body.update(dt);
    }
}

void PhysicsEngine::computeForces() {
    for (auto& body : bodies) {
        // Сброс сил перед новым расчётом
        body.applyForce({0.0f, 0.0f});
    }

    for (size_t i = 0; i < bodies.size(); ++i) {
        for (size_t j = i + 1; j < bodies.size(); ++j) {
            glm::vec2 posA = bodies[i].getPosition();
            glm::vec2 posB = bodies[j].getPosition();
            glm::vec2 direction = posB - posA;
            float distance = glm::length(direction);
            if (distance < 1.0f) distance = 1.0f; // избежать деления на ноль

            glm::vec2 forceDir = glm::normalize(direction);
            float forceMagnitude = G * bodies[i].getMass() * bodies[j].getMass() / (distance * distance);
            glm::vec2 force = forceDir * forceMagnitude;

            bodies[i].applyForce(force);
            bodies[j].applyForce(-force); // по третьему закону Ньютона
        }
    }
}
