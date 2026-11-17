#pragma once
#include "glew/glew.h"
#include "glfw/glfw3.h"

#include "glm/glm.hpp"

// Globals are only declared here, for their initial values go to globals.cpp

// Starile jocului
const int STATE_GALLERY = 0;
const int STATE_PAINTING = 1;

// Constante
const int paintingCount = 3;
const int enemyCount = 3;
const int splatCount = 8;
const int circleSegments = 24;

const float enemyMaxHealth = 60.0f;
const float enemySpeed = 0.25f;
const float splatDuration = 0.6f;
const float levelWonDelay = 1.5f;

// Window
extern GLFWwindow* window;
extern int windowWidth;
extern int windowHeight;
extern float aspect;
extern glm::mat4 projection;

// Jucator
extern glm::vec2 playerPos;
extern float playerSpeed;
extern float playerRadius;
extern float playerHealth;
extern float playerMaxHealth;
extern glm::vec2 cursorPos;

// Lovitura cu pensula
extern bool isSwinging;
extern float swingStartTime;
extern float swingDuration;
extern float swingCosHalfAngle;
extern float swingHalfAngleDeg;
extern float swingRadius;
extern bool swingHitPending;

// Timpul dintre cadrul curent si cel anterior
extern float deltaTime;
extern float lastFrame;

// Niveluri si tablouri
extern int gameState;
extern int currentLevel;
extern int unlockedLevel;
extern bool paintingDone[paintingCount];
extern const char* paintingFiles[paintingCount];
extern const char* paintingGrayFiles[paintingCount];
extern GLuint paintingTex[paintingCount];
extern GLuint paintingGrayTex[paintingCount];
extern const float paintingRatio[paintingCount];

// Culorile cernelii si pensula
extern const glm::vec3 inkColors[3];
extern const int inkStrength[3];
extern const int levelInkColors[paintingCount][3];
extern int brushIndex;

// Programele shader si VAO-urile
extern GLuint programID;
extern GLuint texProgramID;
extern GLuint squareVao;
extern GLuint texVao;
extern GLuint circleVao;
extern GLuint triangleVao;

// Inamici
extern const glm::vec2 enemyStartPos[enemyCount];
extern glm::vec2 enemyPos[enemyCount];
extern float enemyHealth[enemyCount];
extern int enemyColorIndex[enemyCount];

// Pete de vopsea
extern glm::vec2 splatPos[splatCount];
extern float splatStart[splatCount];
extern glm::vec3 splatColor[splatCount];
extern bool splatActive[splatCount];
extern int nextSplat;

// Progresul nivelului
extern float paintingColorAmount;
extern bool levelWon;
extern float levelWonTime;
