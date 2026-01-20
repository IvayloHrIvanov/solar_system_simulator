#include "CelestialBody.h"

using namespace std;

CelestialBody::CelestialBody(glm::vec3 pos, glm::vec3 vel, float massValue, float radiusValue, glm::vec3 col,
                            string n, bool staticBody, CelestialBody* parent)
                            : position(pos), velocity(vel), mass(massValue), radius(radiusValue), color(col), name(n),
                            isStatic(staticBody), parentBody(parent) {
    acceleration = glm::vec3(0.0f);
    rotationAngle = 0.0f;
    rotationSpeed = 0.5f;
    rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    hasTexture = false;
    textureID = 0;
    isOrbitingParent = (parent != nullptr);

    isInShadow = false;
    shadowIntensity = 1.0f;
    shadowDirection = glm::vec3(0.0f);

    isColliding = false;
    collisionTimer = 0.0f;
    originalRadius = radius;
    originalColor = color;

    hasRings = false;
    ringInnerRadius = 1.2f;
    ringOuterRadius = 2.5f;
    ringTextureID = 0;
}

void CelestialBody::updatePosition(float deltaTime) {
    if (isStatic) return;

    updateCollisionAnimation(deltaTime);

    // Using semi - implicit Euler integration for orbital stability
    velocity += acceleration * deltaTime;
    position += velocity * deltaTime;

    rotationAngle += rotationSpeed * deltaTime;
    if (rotationAngle > 360.0f) rotationAngle -= 360.0f;

    addOrbitPoint();
}

void CelestialBody::resetAcceleration() {
    acceleration = glm::vec3(0.0f);
}

void CelestialBody::addOrbitPoint() {
    if (isStatic || name == "Sun") return;

    orbitPoints.push_back(position);

    // Trail orbit length
    if (orbitPoints.size() > 4000) {
        orbitPoints.erase(orbitPoints.begin());
    }
}

void CelestialBody::clearOrbit() {
    orbitPoints.clear();
}

glm::mat4 CelestialBody::getModelMatrix() {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    
    if (rotationSpeed > 0.0f && glm::length(rotationAxis) > 0.0f) {
        model = glm::rotate(model, glm::radians(rotationAngle), rotationAxis);
    }

    model = glm::scale(model, glm::vec3(radius));
    return model;
}

void CelestialBody::startCollisionAnimation() {
    isColliding = true;
    collisionTimer = 2.0f; // 2 second animation
    originalRadius = radius;
}

// Collision animation
void CelestialBody::updateCollisionAnimation(float deltaTime) {
    if (isColliding) {
        collisionTimer -= deltaTime;

        if (collisionTimer > 0) {
            // Pulsing effect: size oscillates during collision
            float pulse = sin(collisionTimer * 20.0f) * 0.2f + 1.0f; // Pulsating size increase

            radius = originalRadius * pulse;

            // Color shift to red during collision, then back to original
            color = glm::mix(glm::vec3(1.0f, 0.3f, 0.3f), originalColor, 1.0f - collisionTimer);
        }
        else {
            // End of animation
            isColliding = false;
            radius = originalRadius;
            color = originalColor;
        }
    }
}