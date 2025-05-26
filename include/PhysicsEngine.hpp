#pragma once
#include <vector>
#include "Body.hpp"

class PhysicsEngine {
public:
    void addBody(const Body& body);
    void update(float dt);

    std::vector<Body>& getBodies();

private:
    std::vector<Body> bodies;
    void computeForces();
};
