#include "globals.hpp"

// Window
GLFWwindow* window = nullptr;

int windowWidth = 800;
int windowHeight = 800;
float aspect = 1.0f;
glm::mat4 projection = glm::mat4(1.0f);

// Jucator
glm::vec2 playerPos = glm::vec2(0.0f, 0.0f);
float playerSpeed = 0.6f;
float playerRadius = 0.1f;
float playerHealth = 100.0f;
float playerMaxHealth = 100.0f;
glm::vec2 cursorPos = glm::vec2(1.0f, 0.0f);

// Lovitura cu pensula
bool isSwinging = false;
float swingStartTime = 0.0f;
float swingDuration = 0.25f;
float swingCosHalfAngle = 0.5f;
float swingHalfAngleDeg = 60.0f;
float swingRadius = 0.45f;
bool swingHitPending = false;

// Timpul dintre cadrul curent si cel anterior
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Niveluri si tablouri
int gameState = STATE_GALLERY;
int currentLevel = 0;
int unlockedLevel = 0;
bool paintingDone[paintingCount] = { false, false, false };
const char* paintingFiles[paintingCount] = {
	"Resources/Images/image1.bmp",
	"Resources/Images/image2.bmp",
	"Resources/Images/image3.bmp"
};
const char* paintingGrayFiles[paintingCount] = {
	"Resources/Images/image1_gray.bmp",
	"Resources/Images/image2_gray.bmp",
	"Resources/Images/image3_gray.bmp"
};
GLuint paintingTex[paintingCount];
GLuint paintingGrayTex[paintingCount];
const float paintingRatio[paintingCount] = { 540.0f / 1024.0f, 557.0f / 1024.0f, 678.0f / 1024.0f };

// Culorile cernelii: albastru, rosu, auriu
const glm::vec3 inkColors[3] = {
	glm::vec3(0.2f, 0.4f, 1.0f),
	glm::vec3(0.9f, 0.15f, 0.15f),
	glm::vec3(0.9f, 0.7f, 0.1f)
};
const int inkStrength[3] = { 1, 2, 3 };
const int levelInkColors[paintingCount][3] = {
	{ 0, 0, 1 },
	{ 1, 2, 0 },
	{ 2, 2, 1 }
};
int brushIndex = 0;

// Programele shader si VAO-urile
GLuint programID;
GLuint texProgramID;
GLuint squareVao;
GLuint texVao;
GLuint circleVao;
GLuint triangleVao;

// Inamici
const glm::vec2 enemyStartPos[enemyCount] = {
	glm::vec2(-0.85f, 0.7f),
	glm::vec2(0.85f, -0.6f),
	glm::vec2(-0.8f, -0.7f)
};
glm::vec2 enemyPos[enemyCount];
float enemyHealth[enemyCount] = { 60.0f, 60.0f, 60.0f };
int enemyColorIndex[enemyCount] = { 0, 0, 0 };

// Pete de vopsea
glm::vec2 splatPos[splatCount];
float splatStart[splatCount];
glm::vec3 splatColor[splatCount];
bool splatActive[splatCount] = {};
int nextSplat = 0;

// Progresul nivelului
float paintingColorAmount = 0.0f;
bool levelWon = false;
float levelWonTime = 0.0f;
