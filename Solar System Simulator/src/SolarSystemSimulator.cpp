#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "CelestialBody.h"
#include "PhysicsEngine.h"
#include "Model.h"
#include "TextureLoader.h"

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

using namespace std;

enum OrbitMode {
    ORBITS_OFF = 0,
    ORBITS_FULL = 1,
    ORBITS_TRAIL = 2
};

// Function declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void generateFullOrbit(CelestialBody* body, CelestialBody* centralBody, vector<glm::vec3>& orbitPoints);
void updateCameraToFollowBody(CelestialBody* body);
void checkManualCameraControl(GLFWwindow* window);
void createBackground();

void loadTextures();

// Settings
const unsigned int SCR_WIDTH = 1500;
const unsigned int SCR_HEIGHT = 800;

// Camera
Camera camera(glm::vec3(0.0f, 10.0f, 30.0f));
float lastMouseX = SCR_WIDTH / 2.0f;
float lastMouseY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

bool cameraFollowMode = false;
bool cameraManualLook = false;
bool cameraManualControl = false;
float minFollowDistance = 2.0f; // Distance from planet when following
float distanceMultiplier = 3.0f; // Distance multiplier based on planet size
float followHeight = 0.5f; // Height above planet plane

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Simulation control
bool simulationRunning = true;
CelestialBody* selectedBody = nullptr;
int selectedBodyIndex = 0;

// Time Control
float timeScale = 1.0f;
const float MIN_TIME_SCALE = 0.1f;
const float MAX_TIME_SCALE = 10.0f;
const float TIME_SCALE_STEP = 0.5f;

// Manual planet movement
bool planetControlMode = false;
float planetMoveSpeed = 5.0f;

// Global objects
vector<CelestialBody*> celestialBodies;
PhysicsEngine physicsEngine;
Model sphereModel, ringModel;

// Shaders
Shader* planetShader = nullptr;
Shader* starShader = nullptr;
Shader* orbitShader = nullptr;
Shader* backgroundShader = nullptr;
Shader* ringShader = nullptr;

// Textures
GLuint sunTexture, mercuryTexture, venusTexture, earthTexture, marsTexture, jupiterTexture, saturnTexture, uranusTexture, neptuneTexture, moonTexture;
GLuint backgroundTexture, backgroundVAO, backgroundVBO;
GLuint saturnRingsTexture;

// Orbit
OrbitMode orbitMode = ORBITS_FULL;

// Create planet, moons and stars bodies
void createSolarSystem() {
    float G = 0.01f; // Gravity strength

    for (auto body : celestialBodies) {
        delete body;
    }
    celestialBodies.clear();

    // Sun
    CelestialBody* sun = new CelestialBody(
        glm::vec3(0.0f, 0.0f, 0.0f), // Initial position
        glm::vec3(0.0f, 0.0f, 0.0f), // Orbital velocity
        10000.0f, // Mass
        3.0f, // Radius
        glm::vec3(1.0f, 0.8f, 0.2f), // Under texture color
        "Sun", // Name
        true // Static 'planet'
    );
    celestialBodies.push_back(sun);

    // Mercury
    glm::vec3 mercuryPos = glm::vec3(11.0f, 0.0f, 0.0f); // Initial position
    float mercuryDistance = glm::length(mercuryPos); // Distance from Sun   
    float mercurySpeed = sqrt(G * sun->mass / mercuryDistance); // Orbital speed using Newton's gravity formula
    glm::vec3 mercuryDir = glm::normalize(glm::cross(mercuryPos, glm::vec3(0.0f, 1.0f, 0.0f))); // Orbital direction
    glm::vec3 mercuryVel = mercuryDir * mercurySpeed; // Orbital velocity

    CelestialBody* mercury = new CelestialBody(
        mercuryPos,
        mercuryVel,
        0.2f,
        0.4f,
        glm::vec3(0.8f, 0.7f, 0.6f),
        "Mercury"
    );
    mercury->rotationSpeed = 15.0f; // Rotation speed 
    celestialBodies.push_back(mercury);

    // Venus
    glm::vec3 venusPos = glm::vec3(18.0f, 0.0f, 1.0f);
    float venusDistance = glm::length(venusPos);
    float venusSpeed = sqrt(G * sun->mass / venusDistance);
    glm::vec3 venusDir = glm::normalize(glm::cross(venusPos, glm::vec3(0.0f, -1.0f, 0.0f))); // Venus rotates backwords
    glm::vec3 venusVel = venusDir * venusSpeed;

    CelestialBody* venus = new CelestialBody(
        venusPos,
        venusVel,
        0.5f,
        0.7f,
        glm::vec3(1.0f, 0.8f, 0.4f),
        "Venus"
    );
    venus->rotationSpeed = 10.0f;
    celestialBodies.push_back(venus);

    // Earth
    glm::vec3 earthPos = glm::vec3(25.0f, 0.0f, 0.0f);
    float earthDistance = glm::length(earthPos);
    float earthSpeed = sqrt(G * sun->mass / earthDistance);
    glm::vec3 earthDir = glm::normalize(glm::cross(earthPos, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 earthVel = earthDir * earthSpeed;

    CelestialBody* earth = new CelestialBody(
        earthPos,
        earthVel,
        2.0f,
        0.8f,
        glm::vec3(0.2f, 0.4f, 1.0f),
        "Earth"
    );
    earth->rotationSpeed = 20.0f;
    celestialBodies.push_back(earth);

    // Mars
    glm::vec3 marsPos = glm::vec3(35.0f, 0.0f, 3.0f);
    float marsDistance = glm::length(marsPos);
    float marsSpeed = sqrt(G * sun->mass / marsDistance);
    glm::vec3 marsDir = glm::normalize(glm::cross(marsPos, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 marsVel = marsDir * marsSpeed;

    CelestialBody* mars = new CelestialBody(
        marsPos,
        marsVel,
        1.2f,
        0.6f,
        glm::vec3(1.0f, 0.3f, 0.2f),
        "Mars"
    );
    mars->rotationSpeed = 20.0f;
    celestialBodies.push_back(mars);

    // Jupiter
    glm::vec3 jupiterPos = glm::vec3(50.0f, 0.0f, -5.0f);
    float jupiterDistance = glm::length(jupiterPos);
    float jupiterSpeed = sqrt(G * sun->mass / jupiterDistance);
    glm::vec3 jupiterDir = glm::normalize(glm::cross(jupiterPos, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 jupiterVel = jupiterDir * jupiterSpeed;

    CelestialBody* jupiter = new CelestialBody(
        jupiterPos,
        jupiterVel,
        7.0f,
        2.0f,
        glm::vec3(0.8f, 0.6f, 0.4f),
        "Jupiter"
    );
    jupiter->rotationSpeed = 40.0f; // Fast rotation
    celestialBodies.push_back(jupiter);

    // Saturn
    glm::vec3 saturnPos = glm::vec3(70.0f, 0.0f, 4.0f);
    float saturnDistance = glm::length(saturnPos);
    float saturnSpeed = sqrt(G * sun->mass / saturnDistance);
    glm::vec3 saturnDir = glm::normalize(glm::cross(saturnPos, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 saturnVel = saturnDir * saturnSpeed;

    CelestialBody* saturn = new CelestialBody(
        saturnPos,
        saturnVel,
        6.0f,
        1.5f,
        glm::vec3(0.9f, 0.8f, 0.6f),
        "Saturn"
    );
    saturn->rotationSpeed = 35.0f;
    saturn->hasRings = true;
    saturn->ringInnerRadius = 1.0f;  // 1.0 times Saturn's radius
    saturn->ringOuterRadius = 2.0f;  // 2.0 times Saturn's radius
    saturn->ringTextureID = saturnRingsTexture;
    celestialBodies.push_back(saturn);

    // Uranus
    glm::vec3 uranusPos = glm::vec3(100.0f, 0.0f, -3.0f);
    float uranusDistance = glm::length(uranusPos);
    float uranusSpeed = sqrt(G * sun->mass / uranusDistance);
    glm::vec3 uranusDir = glm::normalize(glm::cross(uranusPos, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 uranusVel = uranusDir * uranusSpeed;

    CelestialBody* uranus = new CelestialBody(
        uranusPos,
        uranusVel,
        4.0f,
        1.0f,
        glm::vec3(0.6f, 0.8f, 0.9f),
        "Uranus"
    );
    uranus->rotationSpeed = 30.0f;
    uranus->rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f); // Uranus rotates on its side
    celestialBodies.push_back(uranus);

    // Neptune
    glm::vec3 neptunePos = glm::vec3(120.0f, 0.0f, 2.0f);
    float neptuneDistance = glm::length(neptunePos);
    float neptuneSpeed = sqrt(G * sun->mass / neptuneDistance);
    glm::vec3 neptuneDir = glm::normalize(glm::cross(neptunePos, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 neptuneVel = neptuneDir * neptuneSpeed;

    CelestialBody* neptune = new CelestialBody(
        neptunePos,
        neptuneVel,
        5.0f,
        1.2f,
        glm::vec3(0.2f, 0.4f, 0.8f),
        "Neptune"
    );
    neptune->rotationSpeed = 25.0f;
    celestialBodies.push_back(neptune);

    // Moon (orbiting Earth)
    glm::vec3 moonPos = earth->position + glm::vec3(0.0f, 0.0f, 2.0f);
    glm::vec3 moonVel = earth->velocity;

    CelestialBody* moon = new CelestialBody(
        moonPos,
        moonVel,
        0.2f,
        0.3f,
        glm::vec3(0.7f, 0.7f, 0.7f),
        "Moon",
        false,
        earth
    );
    moon->rotationSpeed = 5.0f; // Slow rotation
    celestialBodies.push_back(moon);

    // Assign textures
    celestialBodies[0]->textureID = sunTexture;
    celestialBodies[0]->hasTexture = true;
    celestialBodies[1]->textureID = mercuryTexture;
    celestialBodies[1]->hasTexture = true;
    celestialBodies[2]->textureID = venusTexture;
    celestialBodies[2]->hasTexture = true;
    celestialBodies[3]->textureID = earthTexture;
    celestialBodies[3]->hasTexture = true;
    celestialBodies[4]->textureID = marsTexture;
    celestialBodies[4]->hasTexture = true;
    celestialBodies[5]->textureID = jupiterTexture;
    celestialBodies[5]->hasTexture = true;
    celestialBodies[6]->textureID = saturnTexture;
    celestialBodies[6]->hasTexture = true;
    celestialBodies[7]->textureID = uranusTexture;
    celestialBodies[7]->hasTexture = true;
    celestialBodies[8]->textureID = neptuneTexture;
    celestialBodies[8]->hasTexture = true;
    celestialBodies[9]->textureID = moonTexture;
    celestialBodies[9]->hasTexture = true;

    selectedBody = celestialBodies[0];
}

// Show orbit lines
void createOrbitLines() {
    if (orbitMode == ORBITS_OFF) return;

    orbitShader->use();
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 500.0f);
    orbitShader->setMat4("view", view);
    orbitShader->setMat4("projection", projection);

    for (const auto& body : celestialBodies) {
        // Skip sun and static bodies
        if (body->name == "Sun" || body->isStatic) continue;

        vector<glm::vec3> orbitPointsToCreate;

        // Generate partial orbit
        if (orbitMode == ORBITS_TRAIL) {
            if (body->orbitPoints.size() < 2) continue;

            const int MIN_POINTS = 2000; // Minimum orbit trail distance
            const int START_POINTS = 100; // Orbit trail start distance

            int totalPoints = body->orbitPoints.size();
            orbitPointsToCreate.clear();

            int pointsToShow = min(MIN_POINTS, START_POINTS + (int)((totalPoints / (float)MIN_POINTS) * (MIN_POINTS - START_POINTS)));
            pointsToShow = max(2, pointsToShow);

            for (int i = 0; i < pointsToShow; i++) {
                float t = (float)i / (pointsToShow - 1);
                int srcIdx = (int)(t * (totalPoints - 1));
                srcIdx = min(srcIdx, totalPoints - 1);
                orbitPointsToCreate.push_back(body->orbitPoints[srcIdx]);
            }
        }
        else if (orbitMode == ORBITS_FULL) {
            // Planet orbit
            if (body->parentBody == nullptr) {
                generateFullOrbit(body, celestialBodies[0], orbitPointsToCreate);
            }
            else {
                // Moons orbit
                generateFullOrbit(body, body->parentBody, orbitPointsToCreate);
            }
        }

        if (orbitPointsToCreate.size() < 2) continue;

        GLuint orbitVAO, orbitVBO;
        glGenVertexArrays(1, &orbitVAO);
        glGenBuffers(1, &orbitVBO);

        glBindVertexArray(orbitVAO);
        glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
        glBufferData(GL_ARRAY_BUFFER, orbitPointsToCreate.size() * sizeof(glm::vec3), &orbitPointsToCreate[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        glm::vec3 orbitColor = body->color * 0.7f;
        orbitShader->setVec3("color", orbitColor);

        // Draw orbit line
        glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(orbitPointsToCreate.size()));

        glDeleteVertexArrays(1, &orbitVAO);
        glDeleteBuffers(1, &orbitVBO);
    }
}

// Generates full circular orbits
void generateFullOrbit(CelestialBody* body, CelestialBody* centralBody, vector<glm::vec3>& orbitPoints) {
    orbitPoints.clear();

    // Calculate orbit radius (distance from sun)
    glm::vec3 toBody = body->position - centralBody->position;
    float orbitRadius = glm::length(toBody);

    glm::vec3 normal = glm::normalize(glm::cross(toBody, body->velocity));
    if (glm::length(normal) < 0.1f) {
        normal = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    const int segments = 64;
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * glm::pi<float>() * i / segments;

        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, normal);

        glm::vec3 initialDir = glm::normalize(toBody);
        if (glm::length(glm::cross(initialDir, normal)) < 0.1f) {
            initialDir = glm::vec3(1.0f, 0.0f, 0.0f);
            if (glm::length(glm::cross(initialDir, normal)) < 0.1f) {
                initialDir = glm::vec3(0.0f, 0.0f, 1.0f);
            }
        }

        glm::vec3 orbitDir = glm::vec3(rotation * glm::vec4(initialDir, 0.0f));

        glm::vec3 orbitPoint = centralBody->position + orbitDir * orbitRadius;
        orbitPoints.push_back(orbitPoint);
    }
}

void menu() {
    cout << "Solar System Reset" << endl;
    cout << "\n=== Controls ===" << endl;
    cout << "WASD + Space/Shift: Move camera" << endl;
    cout << "Mouse: Look around" << endl;
    cout << "P: Pause/Resume simulation" << endl;
    cout << "O: Cycle orbit modes" << endl;
    cout << "R: Reset simulation" << endl;

    cout << "\nTAB: Select and auto-follow next planet" << endl;
    cout << "CTRL+TAB: Select and auto-follow previous planet" << endl;
    cout << "F: Auto-follow current planet" << endl;

    cout << "\n+/-: Speed up/Slow down time (0.1x to 10x)" << endl;
    cout << "0: Reset time to normal speed" << endl;

    cout << "\nArrow Keys: Apply horizontal impulse to selected body" << endl;
    cout << "Page Up and Page Down Keys: Apply vertical impulse to selected body" << endl;
    cout << "Backspace: Stop selected body" << endl;
    cout << "================\n" << endl;
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        cout << "Failed to initialize GLFW" << endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System Project", NULL, NULL);
    
    if (window == NULL) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        cout << "Failed to initialize GLEW: " << glewGetErrorString(glewError) << endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    cout << "Loading shaders..." << endl;

    try {
        backgroundShader = new Shader("../shaders/background.vertex", "../shaders/background.fragment");
        cout << "Background shader loaded successfully" << endl;

        starShader = new Shader("../shaders/star.vertex", "../shaders/star.fragment");
        cout << "Star shader loaded successfully" << endl;

        planetShader = new Shader("../shaders/planet.vertex", "../shaders/planet.fragment");
        cout << "Planet shader loaded successfully" << endl;

        orbitShader = new Shader("../shaders/orbit.vertex", "../shaders/orbit.fragment");
        cout << "Orbit shader loaded successfully" << endl;

        ringShader = new Shader("../shaders/ring.vertex", "../shaders/ring.fragment");
        cout << "Ring shader loaded successfully" << endl;
    }
    catch (const exception& e) {
        cout << "Shader loading failed: " << e.what() << endl;
        return -1;
    }

    // Load textures
    loadTextures();
    // Load background texture
    createBackground();

    // Create sphere model
    sphereModel.createSphere(1.0f, 64, 64);
    // Create ring model
    ringModel.createRing(1.0f, 2.5f, 64);

    // Create solar system
    createSolarSystem();
    menu();

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;

        lastFrame = currentFrame;

        processInput(window);
        checkManualCameraControl(window);

        if (cameraFollowMode && selectedBody) {
            updateCameraToFollowBody(selectedBody);
        }

        glClearColor(0.02f, 0.02f, 0.05f, 1.0f); // Default dark blue background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Create star background
        glDisable(GL_DEPTH_TEST);
        backgroundShader->use();
        glBindVertexArray(backgroundVAO);
        glBindTexture(GL_TEXTURE_2D, backgroundTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEnable(GL_DEPTH_TEST);

        if (simulationRunning) {
            int physicsSubsteps = 4;
            float substepDelta = (deltaTime * timeScale) / physicsSubsteps;

            for (int i = 0; i < physicsSubsteps; i++) {
                physicsEngine.updatePhysics(celestialBodies, substepDelta);
            }
        }

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 500.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::vec3 sunPosition = celestialBodies[0]->position;

        // Create rings with shaders
        for (const auto& body : celestialBodies) {
            if (body->hasRings && saturnRingsTexture != 0) {
                ringShader->use();

                glm::mat4 ringModelMatrix = glm::mat4(1.0f);
                ringModelMatrix = glm::translate(ringModelMatrix, body->position);

                // Rings tilt
                ringModelMatrix = glm::rotate(ringModelMatrix, glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.5f));

                float ringScale = body->ringOuterRadius;
                ringModelMatrix = glm::scale(ringModelMatrix,
                    glm::vec3(ringScale, 0.001f, ringScale)); // Rings thickness

                ringShader->setMat4("model", ringModelMatrix);
                ringShader->setMat4("view", view);
                ringShader->setMat4("projection", projection);
                ringShader->setVec3("color", glm::vec3(1.0f, 1.0f, 1.0f));
                ringShader->setBool("useTexture", true);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, saturnRingsTexture);
                ringShader->setInt("textureSampler", 0);

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                ringModel.create();

                glDisable(GL_BLEND);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }

        // Create planet bodies with shaders
        for (const auto& body : celestialBodies) {
            if (body->name == "Sun") {
                starShader->use();
                starShader->setMat4("projection", projection);
                starShader->setMat4("view", view);
                starShader->setMat4("model", body->getModelMatrix());
                starShader->setVec3("color", body->color);
                starShader->setBool("useTexture", body->hasTexture);

                if (body->hasTexture) {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, body->textureID);
                    starShader->setInt("textureSampler", 0);
                }
            }
            else {
                planetShader->use();
                planetShader->setMat4("projection", projection);
                planetShader->setMat4("view", view);
                planetShader->setMat4("model", body->getModelMatrix());
                planetShader->setVec3("color", body->color);
                planetShader->setVec3("lightPos", sunPosition);
                planetShader->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 0.9f));
                planetShader->setVec3("viewPos", camera.Position);
                planetShader->setBool("useTexture", body->hasTexture);
                planetShader->setBool("inShadow", body->isInShadow);
                planetShader->setFloat("shadowIntensity", body->shadowIntensity);
                planetShader->setVec3("shadowDirection", body->shadowDirection);

                if (body->hasTexture) {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, body->textureID);
                    planetShader->setInt("textureSampler", 0);
                }
            }

            sphereModel.create();
        }

        // Create orbit lines
        createOrbitLines();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    for (auto body : celestialBodies) {
        delete body;
    }
    delete planetShader;
    delete starShader;
    delete orbitShader;
    delete backgroundShader;

    glDeleteTextures(1, &sunTexture);
    glDeleteTextures(1, &venusTexture);
    glDeleteTextures(1, &mercuryTexture);
    glDeleteTextures(1, &earthTexture);
    glDeleteTextures(1, &marsTexture);
    glDeleteTextures(1, &jupiterTexture);
    glDeleteTextures(1, &saturnTexture);
    glDeleteTextures(1, &saturnRingsTexture);
    glDeleteTextures(1, &uranusTexture);
    glDeleteTextures(1, &neptuneTexture);
    glDeleteTextures(1, &moonTexture);
    glDeleteTextures(1, &backgroundTexture);

    glDeleteVertexArrays(1, &backgroundVAO);
    glDeleteBuffers(1, &backgroundVBO);

    glfwTerminate();
    return 0;
}

// Load texture files
void loadTextures() {
    cout << "Loading textures..." << endl;

    backgroundTexture = loadTextureFromFile("../textures/background.jpg");
    sunTexture = loadTextureFromFile("../textures/sun.jpg");
    mercuryTexture = loadTextureFromFile("../textures/mercury.jpg");
    venusTexture = loadTextureFromFile("../textures/venus.jpg");
    earthTexture = loadTextureFromFile("../textures/earth.jpg");
    marsTexture = loadTextureFromFile("../textures/mars.jpg");
    jupiterTexture = loadTextureFromFile("../textures/jupiter.jpg");
    saturnTexture = loadTextureFromFile("../textures/saturn.jpg");
    saturnRingsTexture = loadTextureFromFile("../textures/saturn_rings.png");
    uranusTexture = loadTextureFromFile("../textures/uranus.jpg");
    neptuneTexture = loadTextureFromFile("../textures/neptune.jpg");
    moonTexture = loadTextureFromFile("../textures/moon.jpg");

    cout << "All textures loaded!" << endl;
}

// Handle mouse movement
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastMouseX;
    float yoffset = lastMouseY - ypos;
    lastMouseX = xpos;
    lastMouseY = ypos;

    if (abs(xoffset) > 1.0f || abs(yoffset) > 1.0f) {
        cameraManualLook = true;
    }

    if (cameraManualLook) {
        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        camera.Yaw += xoffset;
        camera.Pitch += yoffset;

        if (camera.Pitch > 89.0f) camera.Pitch = 89.0f;
        if (camera.Pitch < -89.0f) camera.Pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(camera.Yaw)) * cos(glm::radians(camera.Pitch));
        front.y = sin(glm::radians(camera.Pitch));
        front.z = sin(glm::radians(camera.Yaw)) * cos(glm::radians(camera.Pitch));
        camera.Front = glm::normalize(front);

        camera.Right = glm::normalize(glm::cross(camera.Front, camera.WorldUp));
        camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));
    }
}

void resetCameraLook() {
    cameraManualLook = false;
    firstMouse = true;
}

// Handle mouse wheel
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// Handle window resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Process camera button clicks
void processInput(GLFWwindow* window) {
    // Escape Key
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // W Key
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.ProcessKeyboard(FORWARD, deltaTime);
        cameraManualControl = true;
    }

    // S Key
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
        cameraManualControl = true;
    }

    // A Key
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.ProcessKeyboard(LEFT, deltaTime);
        cameraManualControl = true;
    }

    // D Key
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(RIGHT, deltaTime);
        cameraManualControl = true;
    }

    // Space Key
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera.ProcessKeyboard(UP, deltaTime);
        cameraManualControl = true;
    }

    // Shift Key
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camera.ProcessKeyboard(DOWN, deltaTime);
        cameraManualControl = true;
    }
}

// Process simulation controls button clicks
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        float impulseStrength = 1.0f;

        switch (key) {

        // P Key
        case GLFW_KEY_P:
            simulationRunning = !simulationRunning;
            cout << "Simulation " << (simulationRunning ? "Resumed" : "Paused") << endl;
            break;

        // O Key
        case GLFW_KEY_O:
            // Cycle through orbit modes: Trail -> Full -> Off -> Trail...
            orbitMode = static_cast<OrbitMode>((orbitMode + 1) % 3);

            switch (orbitMode) {
            case ORBITS_OFF:
                cout << "Orbits: OFF" << endl;
                break;
            case ORBITS_TRAIL:
                cout << "Orbits: TRAIL (showing path history)" << endl;
                break;
            case ORBITS_FULL:
                cout << "Orbits: FULL (showing complete orbits)" << endl;
                break;
            }
            break;

        // R Key
        case GLFW_KEY_R:
            createSolarSystem();
            timeScale = 1.0f;
            selectedBodyIndex = 0;

            #ifdef _WIN32
                system("cls");
            #else
                system("clear");
            #endif

            menu();
            break;

        // TAB Key
        case GLFW_KEY_TAB:
            // CTRL Key
            if (mods & GLFW_MOD_CONTROL) {
                // CTRL + TAB: Go to previous body
                selectedBodyIndex = (selectedBodyIndex - 1 + (int)celestialBodies.size()) % (int)celestialBodies.size();
            }
            else {
                // TAB: Go to next body
                selectedBodyIndex = (selectedBodyIndex + 1) % celestialBodies.size();
            }
            selectedBody = celestialBodies[selectedBodyIndex];

            // Enable camera follow mode automatically when selecting a body
            cameraFollowMode = true;
            cameraManualControl = false;
            resetCameraLook();

            if (selectedBody->name != "Sun") {
                cout << "Selected: " << selectedBody->name << " (Camera following)" << endl;
            }
            else {
                cout << "Selected: " << selectedBody->name << endl;
            }
            break;

        // F Key
        case GLFW_KEY_F:
            // F: Reset Auto-Follow mode
            if (!cameraManualLook && !cameraManualControl) {
                break;
            }

            cameraManualControl = false;
            cameraManualLook = false;
            firstMouse = true;

            if (cameraFollowMode && selectedBody) {
                updateCameraToFollowBody(selectedBody);
            }

            if (selectedBody && selectedBody->name != "Sun") {
                cout << "Camera reset to auto-follow" << endl;
            }
            break;

        // Arrow UP Key
        case GLFW_KEY_UP:
            if (selectedBody && !selectedBody->isStatic && selectedBody->name != "Sun") {
                // UP: Push away from Sun (accumulate)
                glm::vec3 toSun = celestialBodies[0]->position - selectedBody->position;
                glm::vec3 sunDir = glm::normalize(toSun);
                selectedBody->velocity += sunDir * 1.5f; // Power of pull
                cout << "Pulling " << selectedBody->name << " toward Sun" << endl;
            }
            break;

        // Arrow DOWN Key
        case GLFW_KEY_DOWN:
            if (selectedBody && !selectedBody->isStatic && selectedBody->name != "Sun") {
                // DOWN: Pull toward Sun (accumulate)
                glm::vec3 toSun = celestialBodies[0]->position - selectedBody->position;
                glm::vec3 sunDir = glm::normalize(toSun);
                selectedBody->velocity -= sunDir * 1.0f; // Power of push
                cout << "Pushing " << selectedBody->name << " away from Sun" << endl;
            }
            break;

        // Arrow LEFT Key
        case GLFW_KEY_LEFT:
            if (selectedBody && !selectedBody->isStatic && selectedBody->name != "Sun") {
                // LEFT: Add counter-clockwise orbital velocity (accumulate)
                glm::vec3 toSun = celestialBodies[0]->position - selectedBody->position;
                glm::vec3 tangentDir = glm::normalize(glm::cross(toSun, glm::vec3(0.0f, 1.0f, 0.0f)));
                selectedBody->velocity -= tangentDir * 0.3f; // Negative for counter-clockwise

                // Calculate current orbital speed direction
                glm::vec3 orbitalVel = selectedBody->velocity - celestialBodies[0]->velocity;
                float tangentSpeed = glm::dot(orbitalVel, tangentDir);

                cout << "Adding counter-clockwise spin to " << selectedBody->name << endl;
            }
            break;

        // Arrow RIGHT Key
        case GLFW_KEY_RIGHT:
            if (selectedBody && !selectedBody->isStatic && selectedBody->name != "Sun") {
                // RIGHT: Add clockwise orbital velocity (accumulate)
                glm::vec3 toSun = celestialBodies[0]->position - selectedBody->position;
                glm::vec3 tangentDir = glm::normalize(glm::cross(toSun, glm::vec3(0.0f, 1.0f, 0.0f)));
                selectedBody->velocity += tangentDir * 0.3f; // Positive for clockwise

                // Calculate current orbital speed direction
                glm::vec3 orbitalVel = selectedBody->velocity - celestialBodies[0]->velocity;
                float tangentSpeed = glm::dot(orbitalVel, tangentDir);

                cout << "Adding clockwise spin to " << selectedBody->name << endl;
            }
            break;

        // Page UP Key
        case GLFW_KEY_PAGE_UP:
            if (selectedBody && !selectedBody->isStatic) {
                // Page Up: Apply impulse in camera up direction
                glm::vec3 impulse = camera.Up * impulseStrength;
                selectedBody->velocity += impulse;
                cout << "Applied camera-up impulse to " << selectedBody->name << endl;
            }
            break;

        // Page DOWN Key
        case GLFW_KEY_PAGE_DOWN:
            if (selectedBody && !selectedBody->isStatic) {
                // Page Down: Apply impulse in camera down direction
                glm::vec3 impulse = -camera.Up * impulseStrength;
                selectedBody->velocity += impulse;
                cout << "Applied camera-down impulse to " << selectedBody->name << endl;
            }
            break;

        // Backspace Key
        case GLFW_KEY_BACKSPACE:
            if (selectedBody && !selectedBody->isStatic) {
                selectedBody->velocity = glm::vec3(0.0f);
                cout << "Stopped " << selectedBody->name << endl;
            }
            break;

        // + Key
        case GLFW_KEY_KP_ADD:
            if (timeScale != MIN_TIME_SCALE) {
                timeScale = min(timeScale + TIME_SCALE_STEP, MAX_TIME_SCALE);
            }
            else {
                timeScale = min(timeScale + (TIME_SCALE_STEP - 0.1f), MAX_TIME_SCALE);
            }

            cout << "Time speed: " << timeScale << "x" << endl;
            break;

        // - Key
        case GLFW_KEY_KP_SUBTRACT:
            timeScale = max(timeScale - TIME_SCALE_STEP, MIN_TIME_SCALE);
            cout << "Time speed: " << timeScale << "x" << endl;
            break;

        // 0 Key
        case GLFW_KEY_0:
        case GLFW_KEY_KP_0:
            timeScale = 1.0f;
            cout << "Time speed reset to: " << timeScale << "x" << endl;
            break;
        }
    }
}

// Update camera to follow planet body
void updateCameraToFollowBody(CelestialBody* body) {
    if (!body || !cameraFollowMode) return;

    // Special handling for Sun
    if (body->name == "Sun") {
        if (!cameraManualControl) {
            camera.Position = body->position + glm::vec3(0.0f, 8.0f, 25.0f);
        }

        // Camera position to look at Sun
        if (!cameraManualLook) {
            camera.Front = glm::normalize(body->position - camera.Position);
            camera.Right = glm::normalize(glm::cross(camera.Front, camera.WorldUp));
            camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));
            camera.Yaw = glm::degrees(atan2(camera.Front.z, camera.Front.x));
            camera.Pitch = glm::degrees(asin(camera.Front.y));
        }
        return;
    }

    // Special handling for Moon
    if (body->name == "Moon" && body->parentBody) {
        glm::vec3 moonToEarth = body->parentBody->position - body->position;
        glm::vec3 earthDir = glm::normalize(moonToEarth);

        float scaledFollowDistance = 2.0f + (body->radius * 2.0f);
        float scaledFollowHeight = followHeight + (body->radius * 0.5f);

        glm::vec3 cameraOffset = -earthDir * scaledFollowDistance;
        cameraOffset.y += scaledFollowHeight;

        if (!cameraManualControl) {
            camera.Position = body->position + cameraOffset;
        }

        // Camera position to look at Moon
        if (!cameraManualLook) {
            glm::vec3 desiredFront = glm::normalize(body->position - camera.Position);
            camera.Front = desiredFront;
            camera.Right = glm::normalize(glm::cross(camera.Front, camera.WorldUp));
            camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));
            camera.Yaw = glm::degrees(atan2(camera.Front.z, camera.Front.x));
            camera.Pitch = glm::degrees(asin(camera.Front.y));
        }
        return;
    }

    glm::vec3 toSun = celestialBodies[0]->position - body->position;
    glm::vec3 orbitalDir = glm::normalize(glm::cross(toSun, glm::vec3(0.0f, 1.0f, 0.0f)));

    if (glm::length(orbitalDir) < 0.1f) {
        orbitalDir = glm::vec3(0.0f, 0.0f, 1.0f);
    }

    float scaledFollowDistance = minFollowDistance + (body->radius * distanceMultiplier);
    float scaledFollowHeight = followHeight + (body->radius * 0.5f);

    glm::vec3 cameraOffset = glm::vec3(-scaledFollowDistance, scaledFollowHeight, 0.0f);

    if (!cameraManualControl) {
        camera.Position = body->position + cameraOffset;
    }

    // Camera position to look at planet
    if (!cameraManualLook) {
        camera.Front = glm::normalize(body->position - camera.Position);
        camera.Right = glm::normalize(glm::cross(camera.Front, camera.WorldUp));
        camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));
        camera.Yaw = glm::degrees(atan2(camera.Front.z, camera.Front.x));
        camera.Pitch = glm::degrees(asin(camera.Front.y));
    }
}

// Check if user is manually controlling camera
void checkManualCameraControl(GLFWwindow* window) {
    // Check for any camera movement keys
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        cameraManualControl = true;
        cameraManualLook = true;
    }
}

// Background geometry: Makes the background on fullscreen and covers the entire viewport
void createBackground() {
    float quadVertices[] = {
  // Corner positions, // Image coordinatess
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    // Each line defines one corner of the rectangle with : [X position, Y position, Texture X, Texture Y]. 6 points make two triangles

    glGenVertexArrays(1, &backgroundVAO);
    glGenBuffers(1, &backgroundVBO);
    glBindVertexArray(backgroundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, backgroundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}