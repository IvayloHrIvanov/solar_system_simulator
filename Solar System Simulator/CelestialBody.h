#ifndef CELESTIALBODY_H
#define CELESTIALBODY_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>

using namespace std;

class CelestialBody {
public:
    // Physical properties
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    float mass;
    float radius;

    // Visual properties
    glm::vec3 color;
    string name;
    bool hasTexture;
    GLuint textureID;

    // Simulation properties
    bool isStatic;
    float rotationAngle;
    float rotationSpeed;
    glm::vec3 rotationAxis;

    // Orbit tracking
    vector<glm::vec3> orbitPoints;

    CelestialBody* parentBody;  // For moons - which planet they orbit
    bool isOrbitingParent;

    // Shadow properties
    bool isInShadow;
    float shadowIntensity;
    glm::vec3 shadowDirection;

    // Collision properties
    bool isColliding;
    float collisionTimer;
    float originalRadius;
    glm::vec3 originalColor;
    glm::vec3 orbitColor;

    // Ring properties
    bool hasRings;
    float ringInnerRadius;
    float ringOuterRadius;
    GLuint ringTextureID;

    CelestialBody(glm::vec3 pos, glm::vec3 vel, float m, float r, glm::vec3 col,
                  string n, bool staticBody = false, CelestialBody* parent = nullptr);

    void updatePosition(float deltaTime);
    void resetAcceleration();
    void addOrbitPoint();
    void clearOrbit();

    glm::mat4 getModelMatrix();

    void startCollisionAnimation();
    void updateCollisionAnimation(float deltaTime);
};

#endif