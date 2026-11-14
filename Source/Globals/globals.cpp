#include "globals.hpp"

GLFWwindow* window = nullptr;

int windowWidth = 1024;
int windowHeight = 768;
float aspect = 1024.0f / 768.0f;

glm::vec2 playerPos = glm::vec2(0.0f, 0.0f);
float playerSpeed = 0.6f;
float playerRadius = 0.1f;
float playerHealth = 100.0f;
float playerMaxHealth = 100.0f;
glm::vec3 brushColor =glm::vec3(0.2f, 0.4f, 1.0f);
glm::vec2 cursorPos = glm::vec2(1.0f, 0.0f);

bool isSwinging = false;
float swingStartTime = 0.0f;
float swingDuration = 0.25f;
float swingHalfAngle = 1.0472f;
float swingRadius = 0.45f;
float damageBase = 50.0f;
bool swingHitPending = false;

glm::vec3 backgroundColor = glm::vec3(0.0f, 0.0f, 0.4f);


float deltaTime = 0.0f;
float lastFrame = 0.0f;
