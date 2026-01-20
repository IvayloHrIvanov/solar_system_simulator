#include "PhysicsEngine.h"
#include <GLFW/glfw3.h>
#include <iostream>

using namespace std;

PhysicsEngine::PhysicsEngine() {}

void PhysicsEngine::updatePhysics(vector<CelestialBody*>& bodies, float deltaTime)
{
    for (auto& b : bodies) {
        b->resetAcceleration();
        b->isInShadow = false;
        b->shadowIntensity = 1.0f; // Full brightness by default
    }

    CelestialBody* moon = nullptr;
    CelestialBody* earth = nullptr;
    CelestialBody* sun = nullptr;

    const float G = 0.01f; // Gravity strength
    const float moonEscapeDistance = 5.0f; // Distance from Earth before Moon fills Sun's gravity

    for (auto& body : bodies) {
        if (body->name == "Moon") moon = body;
        if (body->name == "Earth") earth = body;
        if (body->name == "Sun") sun = body;

        if (moon && earth && sun) break;
    }

    // This changes normal gravity for the Moon to ensure stable orbit around Earth
    if (moon && earth) {
        glm::vec3 toEarth = earth->position - moon->position;
        float distToEarth = glm::length(toEarth);

        if (distToEarth <= moonEscapeDistance) {
            float desiredOrbitRadius = 2.0f; // Orbit radius around Earth

            if (distToEarth > 0.01f) {
                glm::vec3 forceDir = glm::normalize(toEarth);

                float orbitForce = 1.0f * (distToEarth - desiredOrbitRadius);

                glm::vec3 currentVelRelative = moon->velocity - earth->velocity;
                glm::vec3 tangentDir = glm::normalize(glm::cross(toEarth, glm::vec3(0.0f, 1.0f, 0.0f)));
                float currentTangential = glm::dot(currentVelRelative, tangentDir);
                float desiredTangential = 1.0f; // Orbital speed around Earth

                float tangentialForce = 1.5f * (desiredTangential - currentTangential);

                moon->acceleration += forceDir * orbitForce;
                moon->acceleration += tangentDir * tangentialForce;
            }
        }
    }

    // Normal gravity
    for (int i = 0; i < bodies.size(); i++) {
        CelestialBody* A = bodies[i];

        for (int j = 0; j < bodies.size(); j++) {
            if (i == j) continue;
            CelestialBody* B = bodies[j];

            if (A->name == "Moon" && earth && glm::length(A->position - earth->position) <= moonEscapeDistance && (B->name == "Earth" || B->name == "Sun")) { continue; }
            if (B->name == "Moon" && earth && glm::length(B->position - earth->position) <= moonEscapeDistance && (A->name == "Earth" || A->name == "Sun")) { continue; }

            glm::vec3 dir = B->position - A->position;
            float dist = glm::length(dir);

            if (dist < 0.01f) { continue; }

            glm::vec3 forceDir = glm::normalize(dir);

            float force = G * A->mass * B->mass / (dist * dist);
            float distanceMultiplier = 1.0f / (dist * 0.005f); // Greater pull strength the closer planets get (the smaller the number)

            if (A->name != "Sun" && B->name != "Sun") {
                if (dist < 7.0f) {
                    if (A->mass < B->mass) {
                        glm::vec3 accelerationA = forceDir * (force / A->mass) * distanceMultiplier;
                        A->acceleration += accelerationA;
                    }
                    else {
                        glm::vec3 accelerationB = -forceDir * (force / B->mass) * distanceMultiplier;
                        B->acceleration += accelerationB;
                    }
                }
                else {
                    glm::vec3 accelerationA = forceDir * (force / A->mass);
                    glm::vec3 accelerationB = -forceDir * (force / B->mass);

                    A->acceleration += accelerationA;
                    B->acceleration += accelerationB;
                }
            }
            else {
                glm::vec3 accelerationA = forceDir * (force / A->mass);
                A->acceleration += accelerationA;
            }
        }
    }

    handleCollisions(bodies);

    // Check for eclipse (Moon between Sun and Earth)
    if (sun && earth && moon) {
        checkForEclipse(sun, earth, moon);
    }

    for (auto& b : bodies)
        b->updatePosition(deltaTime);
}

// Check if a Moon is between planet and Sun and apply a shadow
void PhysicsEngine::checkForEclipse(CelestialBody* sun, CelestialBody* earth, CelestialBody* moon)
{
    earth->isInShadow = false;
    earth->shadowIntensity = 1.0f;

    glm::vec3 sunToEarth = earth->position - sun->position;
    float sunEarthDistance = glm::length(sunToEarth);
    glm::vec3 sunToEarthDir = glm::normalize(sunToEarth);

    glm::vec3 sunToMoon = moon->position - sun->position;

    float projection = glm::dot(sunToMoon, sunToEarthDir);

    // Check if Moon is between Sun and Earth
    if (projection > -moon->radius && projection < sunEarthDistance + moon->radius) {
        glm::vec3 closestPoint = sun->position + sunToEarthDir * projection;

        float distanceToLine = glm::length(moon->position - closestPoint);

        float sunAngularRadius = sun->radius / glm::length(sunToEarth);
        float moonAngularRadius = moon->radius / glm::length(moon->position - earth->position);

        // Calculate shadow parameters based on alignment quality
        float maxShadowDistance = moon->radius * 2.0f; // Shadow area projected on Earth
        float alignment = 1.0f - glm::clamp(distanceToLine / maxShadowDistance, 0.0f, 1.0f);

        if (alignment > 0.3f) {
            earth->isInShadow = true;

            // Gradual shadow intensity based on alignment quality
            float shadowStrength = (alignment - 0.3f) / 0.7f;
            earth->shadowIntensity = 1.0f - shadowStrength * 0.8f; // Intensity/Saturaion of the shadow

            earth->shadowDirection = -sunToEarthDir;
        }
    }
}

void PhysicsEngine::handleCollisions(vector<CelestialBody*>& bodies) {
    // Check all pairs of bodies for collisions
    for (int i = 0; i < bodies.size(); i++) {
        for (int j = i + 1; j < bodies.size(); j++) {
            if (checkCollision(bodies[i], bodies[j])) {
                resolveCollision(bodies[i], bodies[j]);
            }
        }
    }
}

bool PhysicsEngine::checkCollision(CelestialBody* a, CelestialBody* b) {
    float distance = glm::length(a->position - b->position);
    float minDistance = a->radius + b->radius;

    float collisionMargin = 0.1f; // Distance before a collision occures
    
    return distance < (minDistance - collisionMargin);
}

void PhysicsEngine::resolveCollision(CelestialBody* a, CelestialBody* b) {
    bool aIsSun = (a->name == "Sun");
    bool bIsSun = (b->name == "Sun");

    // Handle Sun collision specially
    if (aIsSun || bIsSun) {
        CelestialBody* sun = aIsSun ? a : b;
        CelestialBody* planet = aIsSun ? b : a;

        cout << "SUN COLLISION DETECTED with " << planet->name << endl;

        glm::vec3 collisionNormal = glm::normalize(planet->position - sun->position);

        float currentDistance = glm::length(planet->position - sun->position);
        float desiredDistance = sun->radius + planet->radius + 2.0f;
        float penetration = desiredDistance - currentDistance;

        if (penetration > 0) {
            planet->position = sun->position + collisionNormal * desiredDistance;
        }

        glm::vec3 relativeVelocity = planet->velocity - sun->velocity;
        float velocityTowardSun = glm::dot(relativeVelocity, collisionNormal);

        if (velocityTowardSun < 0) {

            planet->velocity -= collisionNormal * velocityTowardSun;

            glm::vec3 tangentDir = glm::normalize(glm::cross(collisionNormal, glm::vec3(0.0f, 1.0f, 0.0f)));
            float orbitSpeed = glm::length(relativeVelocity) * 0.2f; // Collision strength
            planet->velocity += tangentDir * orbitSpeed;

            float maxCollisionSpeed = 10.0f; // Max collision speed with Sun
            if (glm::length(planet->velocity) > maxCollisionSpeed) {
                planet->velocity = glm::normalize(planet->velocity) * maxCollisionSpeed;
            }
        }

        planet->startCollisionAnimation();
        static float sunCollisionCooldown = 0.0f;
        static float lastSunCollisionTime = 0.0f;
        float currentTime = static_cast<float>(glfwGetTime());

        if (currentTime - lastSunCollisionTime < 1.0f) {
            return;
        }
        lastSunCollisionTime = currentTime;

        return;
    }

    // Regular planet collision
    glm::vec3 collisionNormal = glm::normalize(a->position - b->position);
    glm::vec3 relativeVelocity = a->velocity - b->velocity;
    float velocityAlongNormal = glm::dot(relativeVelocity, collisionNormal);

    if (velocityAlongNormal > 0) return;

    float overlap = (a->radius + b->radius) - glm::length(a->position - b->position);

    if (overlap > 0) {
        float totalMass = a->mass + b->mass;
        float aRatio = b->mass / totalMass;
        float bRatio = a->mass / totalMass;

        glm::vec3 separation = collisionNormal * overlap;

        if (!a->isStatic) a->position += separation * aRatio * 0.5f;
        if (!b->isStatic) b->position -= separation * bRatio * 0.5f;

        float safetyMargin = 0.1f;
        if (!a->isStatic) a->position += collisionNormal * safetyMargin * aRatio;
        if (!b->isStatic) b->position -= collisionNormal * safetyMargin * bRatio;
    }

    float restitution = 0.8f;
    float impulseScalar = -(1.0f + restitution) * velocityAlongNormal;
    impulseScalar /= (1.0f / a->mass + 1.0f / b->mass);

    static float lastCollisionTime = 0.0f;
    float currentTime = static_cast<float>(glfwGetTime());
    float timeSinceLastCollision = currentTime - lastCollisionTime;
    lastCollisionTime = currentTime;

    if (timeSinceLastCollision < 0.1f) {
        float dampingFactor = glm::clamp(timeSinceLastCollision / 0.1f, 0.1f, 1.0f);
        impulseScalar *= dampingFactor;
    }

    glm::vec3 impulse = collisionNormal * impulseScalar;

    // Strength of the collision is calculated by the planets mass
    if (!a->isStatic) {
        a->velocity += impulse / a->mass;
    }
    if (!b->isStatic) {
        b->velocity -= impulse / b->mass;
    }

    float maxCollisionSpeed = 10.0f; // Max collision speed between planets

    if (!a->isStatic) {
        float currentSpeed = glm::length(a->velocity);
        if (currentSpeed > maxCollisionSpeed) {
            float damping = 0.7f;
            a->velocity = glm::normalize(a->velocity) * (maxCollisionSpeed * damping);
        }
        else if (currentSpeed > 5.0f) {
            a->velocity *= 0.9f;
        }
    }

    if (!b->isStatic) {
        float currentSpeed = glm::length(b->velocity);
        if (currentSpeed > maxCollisionSpeed) {
            float damping = 0.7f;
            b->velocity = glm::normalize(b->velocity) * (maxCollisionSpeed * damping);
        }
        else if (currentSpeed > 5.0f) {
            b->velocity *= 0.9f;
        }
    }

    glm::vec3 tangentDir = glm::normalize(glm::cross(collisionNormal, glm::vec3(0.0f, 1.0f, 0.0f)));
    if (glm::length(tangentDir) > 0.1f) {
        float spinStrength = 0.5f;
        if (!a->isStatic) a->velocity += tangentDir * spinStrength * (b->mass / a->mass);
        if (!b->isStatic) b->velocity -= tangentDir * spinStrength * (a->mass / b->mass);
    }

    a->startCollisionAnimation();
    b->startCollisionAnimation();

    cout << "PLANET COLLISION: " << a->name << " hit " << b->name << endl;
}