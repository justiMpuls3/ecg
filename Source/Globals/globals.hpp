#pragma once
// Include GLEW and GLFW
#include "glew/glew.h"
#include "glfw/glfw3.h"


#include "glm/glm.hpp"

extern GLFWwindow* window;
extern int windowWidth;
extern int windowHeight;
extern float aspect;

extern glm::vec2 playerPos;
extern float playerSpeed;
extern float playerRadius;
extern float playerHealth;
extern float playerMaxHealth;
extern glm::vec3 brushColor;
extern glm::vec2 cursorPos;

extern bool isSwinging;
extern float swingStartTime;
extern float swingDuration;
extern float swingHalfAngle;
extern float swingRadius;
extern float damageBase;
extern bool swingHitPending;

extern glm::vec3 backgroundColor;


extern float deltaTime;
extern float lastFrame;
