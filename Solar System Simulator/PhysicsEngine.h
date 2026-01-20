#ifndef PHYSICSENGINE_H
#define PHYSICSENGINE_H

#include <vector>
#include <glm/glm.hpp>
#include "CelestialBody.h"

using namespace std;

class PhysicsEngine {
public:
    PhysicsEngine();

    void handleCollisions(vector<CelestialBody*>& bodies);
    bool checkCollision(CelestialBody* a, CelestialBody* b);
    void resolveCollision(CelestialBody* a, CelestialBody* b);
    
    void updatePhysics(vector<CelestialBody*>& bodies, float deltaTime);
    void checkForEclipse(class CelestialBody* sun, class CelestialBody* earth, class CelestialBody* moon);
};

#endif