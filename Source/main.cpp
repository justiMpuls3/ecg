// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Include GLEW and GLFW
#include "glew/glew.h"
#include "glfw/glfw3.h"

// Include GLM
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// Include our helper function for loading shaders
#include "Shaders/Utils/shaderLoader.hpp"

// Functia LoadImage pentru texturi
#include "Texture/texture.hpp"

// Global variables
#include "Globals/globals.hpp"

// Cercurile din care e facut un Inkblot (pozitie si scara)
const glm::vec3 blobPositions[4] = {
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.05f, 0.04f, 0.0f),
	glm::vec3(-0.06f, 0.03f, 0.0f),
	glm::vec3(0.0f, -0.07f, 0.0f)
};
const float blobScales[4] = { 1.6f, 0.9f, 0.8f, 0.7f };

// Cercurile din care e facuta o pata de vopsea
const glm::vec3 splatPartPositions[4] = {
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.06f, 0.05f, 0.0f),
	glm::vec3(-0.07f, 0.02f, 0.0f),
	glm::vec3(0.01f, -0.07f, 0.0f)
};
const float splatPartScales[4] = { 1.0f, 0.5f, 0.4f, 0.45f };

// Centrul ramei i din galerie
glm::vec2 frameCenter(int i)
{
	return glm::vec2((i - 1) * 0.62f, 0.25f);
}

// Latimea si inaltimea tabloului i, cu proportia pozei
glm::vec2 frameSize(int i)
{
	return glm::vec2(0.5f, 0.5f * paintingRatio[i]);
}

// E cursorul in rama i?
bool cursorInFrame(int i)
{
	glm::vec2 c = frameCenter(i);
	glm::vec2 s = frameSize(i);
	return cursorPos.x >= c.x - s.x * 0.5f && cursorPos.x <= c.x + s.x * 0.5f &&
		cursorPos.y >= c.y - s.y * 0.5f && cursorPos.y <= c.y + s.y * 0.5f;
}

// La intrarea in tablou: inamici noi, fara pete, jucatorul in centru
void resetLevel()
{
	for (int i = 0; i < enemyCount; i++)
	{
		enemyPos[i] = enemyStartPos[i];
		enemyHealth[i] = enemyMaxHealth;
		enemyColorIndex[i] = levelInkColors[currentLevel][i];
	}
	for (int i = 0; i < splatCount; i++)
	{
		splatActive[i] = false;
	}
	playerHealth = playerMaxHealth;
	playerPos = glm::vec2(0.0f, 0.0f);
	paintingColorAmount = 0.0f;
	levelWon = false;
	isSwinging = false;
	swingHitPending = false;
}

// Intoarcerea in galerie
void goToGallery()
{
	gameState = STATE_GALLERY;
	playerPos = glm::vec2(0.0f, -0.6f);
	isSwinging = false;
	swingHitPending = false;
}

// Callback function for cursor position
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	// Don't forget that OpenGL coordinates are between -1,-1 and 1,1
	cursorPos.x = (2.0f * (float)xpos / (float)windowWidth - 1.0f) * aspect;
	cursorPos.y = 1.0f - 2.0f * (float)ypos / (float)windowHeight;
}

// Callback function for mouse button
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS)
	{
		return;
	}

	// In galerie dai click pe un tablou ca sa intri in el
	if (gameState == STATE_GALLERY)
	{
		for (int i = 0; i < paintingCount; i++)
		{
			if (i <= unlockedLevel && !paintingDone[i] && cursorInFrame(i))
			{
				currentLevel = i;
				gameState = STATE_PAINTING;
				resetLevel();
			}
		}
	}
	// In tablou, click = lovitura cu pensula
	else if (!isSwinging)
	{
		isSwinging = true;
		swingStartTime = (float)glfwGetTime();
		swingHitPending = true;
	}
}

// Callback for adjusting the viewport when resizing the window
void window_callback(GLFWwindow* window, int new_width, int new_height)
{
	windowWidth = new_width;
	windowHeight = new_height;
	aspect = (float)new_width / (float)new_height;
	glViewport(0, 0, new_width, new_height);
}

// Input de la tastatura
void processInput(GLFWwindow* window)
{
	// ESC inchide fereastra
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	// M te intoarce in galerie
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && gameState == STATE_PAINTING)
	{
		goToGallery();
	}

	// 1/2/3 schimba culoarea pensulei
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) brushIndex = 0;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) brushIndex = 1;
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) brushIndex = 2;

	// Miscare cu WASD
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) playerPos.y += playerSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) playerPos.y -= playerSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) playerPos.x += playerSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) playerPos.x -= playerSpeed * deltaTime;

	// Jucatorul nu poate iesi din fereastra
	if (playerPos.x < -aspect + 0.05f)
	{
		playerPos.x = -aspect + 0.05f;
	}
	if (playerPos.x > aspect - 0.05f)
	{
		playerPos.x = aspect - 0.05f;
	}
	if (playerPos.y < -0.95f)
	{
		playerPos.y = -0.95f;
	}
	if (playerPos.y > 0.95f)
	{
		playerPos.y = 0.95f;
	}
}

// Trimite projection * model si culoarea la shader, apoi deseneaza
void drawShape(GLuint vao, int indexCount, glm::mat4 model, glm::vec4 color)
{
	glm::mat4 matrix = projection * model;

	unsigned int transformLoc = glGetUniformLocation(programID, "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(matrix));

	unsigned int colorLoc = glGetUniformLocation(programID, "color");
	glUniform4fv(colorLoc, 1, glm::value_ptr(color));

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

// Dreptunghi colorat cu centrul c; patratul de baza are latura 0.1, de aici impartirea la 0.1f
void drawRect(glm::vec2 c, glm::vec2 size, glm::vec4 color)
{
	glUseProgram(programID);
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(c.x, c.y, 0.0f));
	model = glm::scale(model, glm::vec3(size.x / 0.1f, size.y / 0.1f, 1.0f));
	drawShape(squareVao, 6, model, color);
}

// Dreptunghi cu textura: amesteca varianta gri cu cea color
void drawTextured(GLuint texture1, GLuint texture2, glm::vec2 c, glm::vec2 size, float colorAmount, float brightness)
{
	// Shaderul cu textura
	glUseProgram(texProgramID);
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(c.x, c.y, 0.0f));
	model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));
	glm::mat4 matrix = projection * model;

	unsigned int transformLoc = glGetUniformLocation(texProgramID, "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(matrix));

	unsigned int colorAmountLoc = glGetUniformLocation(texProgramID, "colorAmount");
	glUniform1f(colorAmountLoc, colorAmount);

	unsigned int brightnessLoc = glGetUniformLocation(texProgramID, "brightness");
	glUniform1f(brightnessLoc, brightness);

	// texture1 = varianta gri, pe unitatea 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1);
	unsigned int texture1Loc = glGetUniformLocation(texProgramID, "texture1");
	glUniform1i(texture1Loc, 0);

	// texture2 = varianta color, pe unitatea 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture2);
	unsigned int texture2Loc = glGetUniformLocation(texProgramID, "texture2");
	glUniform1i(texture2Loc, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(texVao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

// 4 cercuri desenate fata de matricea base (pentru Inkblot sau pata)
void drawBlob(glm::mat4 base, const glm::vec3 positions[], const float scales[], glm::vec4 color)
{
	for (int i = 0; i < 4; i++)
	{
		glm::mat4 model = glm::translate(base, positions[i]);
		model = glm::scale(model, glm::vec3(scales[i], scales[i], 1.0f));
		drawShape(circleVao, circleSegments * 3, model, color);
	}
}

// Inamicul e lovit daca e la cel mult R si in unghiul arcului (dot >= cos)
bool isInSwingArc(glm::vec2 pPlayer, glm::vec2 pEnemy, glm::vec2 dir, float R, float cosHalfAngle)
{
	glm::vec2 toEnemy = pEnemy - pPlayer;
	float dist = sqrt(toEnemy.x * toEnemy.x + toEnemy.y * toEnemy.y);
	if (dist > R)
	{
		return false;
	}
	return dir.x * toEnemy.x / dist + dir.y * toEnemy.y / dist >= cosHalfAngle;
}

// Cu cat are mai putina viata, cu atat e mai mic (intre 0 si 1)
float enemySize(int i)
{
	float health = enemyHealth[i];
	if (health < 0.0f)
	{
		health = 0.0f;
	}
	return health / enemyMaxHealth;
}

// Galeria
void drawGallery()
{
	// Peretele, podeaua si bordura dintre ele
	drawRect(glm::vec2(0.0f, 0.3f), glm::vec2(2.0f * aspect, 1.4f), glm::vec4(0.80f, 0.77f, 0.70f, 1.0f));
	drawRect(glm::vec2(0.0f, -0.7f), glm::vec2(2.0f * aspect, 0.6f), glm::vec4(0.35f, 0.24f, 0.17f, 1.0f));
	drawRect(glm::vec2(0.0f, -0.38f), glm::vec2(2.0f * aspect, 0.05f), glm::vec4(0.25f, 0.17f, 0.12f, 1.0f));

	for (int i = 0; i < paintingCount; i++)
	{
		glm::vec2 c = frameCenter(i);
		glm::vec2 fs = frameSize(i);
		bool available = (i <= unlockedLevel) && !paintingDone[i];
		bool hover = available && cursorInFrame(i);

		// Rama devine aurie cand e cursorul pe ea
		glm::vec4 frameColor = glm::vec4(0.55f, 0.4f, 0.12f, 1.0f);
		if (hover)
		{
			frameColor = glm::vec4(1.0f, 0.85f, 0.3f, 1.0f);
		}
		drawRect(c, fs + glm::vec2(0.08f, 0.08f), frameColor);

		// Tablourile blocate sunt mai intunecate, cele terminate apar color
		float paintingBrightness = 1.0f;
		if (i > unlockedLevel)
		{
			paintingBrightness = 0.4f;
		}
		float paintingColor = 0.0f;
		if (paintingDone[i])
		{
			paintingColor = 1.0f;
		}
		drawTextured(paintingGrayTex[i], paintingTex[i], c, fs, paintingColor, paintingBrightness);
	}
}

// Inamicii vin spre jucator; intoarce true daca l-a atins vreunul
bool updateEnemies()
{
	bool playerHit = false;

	for (int i = 0; i < enemyCount; i++)
	{
		if (enemyHealth[i] <= 0.0f)
		{
			continue;
		}

		// Directia spre jucator, normalizata
		float enemyRadius = 0.09f * enemySize(i);
		glm::vec2 toPlayer = playerPos - enemyPos[i];
		float dist = sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

		// Coliziune cerc-cerc: inamicul vine spre jucator pana il atinge, apoi ii scade viata
		if (dist > enemyRadius + playerRadius)
		{
			enemyPos[i] += glm::normalize(toPlayer) * enemySpeed * deltaTime;
		}
		else
		{
			playerHit = true;
			playerHealth -= 20.0f * deltaTime;
		}
	}

	// La viata 0, jucatorul reapare in centru cu viata plina
	if (playerHealth <= 0.0f)
	{
		playerHealth = playerMaxHealth;
		playerPos = glm::vec2(0.0f, 0.0f);
	}

	return playerHit;
}

// Inkblots
void drawEnemies()
{
	for (int i = 0; i < enemyCount; i++)
	{
		if (enemyHealth[i] <= 0.0f)
		{
			continue;
		}

		glm::mat4 enemyModel = glm::translate(glm::mat4(1.0f), glm::vec3(enemyPos[i].x, enemyPos[i].y, 0.0f));
		enemyModel = glm::scale(enemyModel, glm::vec3(enemySize(i), enemySize(i), 1.0f));

		// Corpul negru, din cercuri
		drawBlob(enemyModel, blobPositions, blobScales, glm::vec4(0.05f, 0.05f, 0.08f, 1.0f));

		// Cercul din mijloc arata culoarea cernelii inamicului
		glm::mat4 model = glm::scale(enemyModel, glm::vec3(0.5f, 0.5f, 1.0f));
		drawShape(circleVao, circleSegments * 3, model, glm::vec4(inkColors[enemyColorIndex[i]], 1.0f));
	}
}

// Tabloul se coloreaza pe masura ce mor inamicii; la final e marcat ca terminat
void updateLevel(float currentFrame)
{
	int killed = 0;
	for (int i = 0; i < enemyCount; i++)
	{
		if (enemyHealth[i] <= 0.0f)
		{
			killed++;
		}
	}

	// Tinta e procentul de inamici invinsi, iar culoarea se apropie de ea treptat
	float targetColor = (float)killed / (float)enemyCount;
	if (paintingColorAmount < targetColor)
	{
		paintingColorAmount += 0.5f * deltaTime;
		if (paintingColorAmount > targetColor)
		{
			paintingColorAmount = targetColor;
		}
	}

	// Dupa ce mor toti, asteptam levelWonDelay secunde si ne intoarcem in galerie
	if (killed == enemyCount && !levelWon)
	{
		levelWon = true;
		levelWonTime = currentFrame;
	}
	if (levelWon && currentFrame - levelWonTime >= levelWonDelay)
	{
		paintingDone[currentLevel] = true;
		if (currentLevel + 1 > unlockedLevel && currentLevel + 1 < paintingCount)
		{
			unlockedLevel = currentLevel + 1;
		}
		goToGallery();
	}
}

// Petele cresc si scad dupa sin, timp de splatDuration secunde
void drawSplats(float currentFrame)
{
	for (int i = 0; i < splatCount; i++)
	{
		if (!splatActive[i])
		{
			continue;
		}
		float splatProgress = (currentFrame - splatStart[i]) / splatDuration;
		if (splatProgress >= 1.0f)
		{
			splatActive[i] = false;
			continue;
		}
		float splatScale = 2.2f * sin(splatProgress * 3.14159265f);

		glm::mat4 splatModel = glm::translate(glm::mat4(1.0f), glm::vec3(splatPos[i].x, splatPos[i].y, 0.0f));
		splatModel = glm::scale(splatModel, glm::vec3(splatScale, splatScale, 1.0f));

		drawBlob(splatModel, splatPartPositions, splatPartScales, glm::vec4(splatColor[i], 1.0f));
	}
}

// Directia de la jucator spre cursor, normalizata
glm::vec2 getAimDir()
{
	return glm::normalize(cursorPos - playerPos);
}

// Matricea jucatorului; corpul, pensula si paleta pornesc de la ea
glm::mat4 getPlayerModel()
{
	return glm::translate(glm::mat4(1.0f), glm::vec3(playerPos.x, playerPos.y, 0.0f));
}

// Restauratorul
void drawPlayer(bool playerHit)
{
	glm::mat4 playerModel = getPlayerModel();

	// Corpul, rosu cat timp e atins de un inamic
	glm::vec4 bodyColor = glm::vec4(0.2f, 0.5f, 0.6f, 1.0f);
	if (playerHit)
	{
		bodyColor = glm::vec4(0.9f, 0.2f, 0.2f, 1.0f);
	}
	glm::mat4 model = glm::scale(playerModel, glm::vec3(1.4f, 1.8f, 1.0f));
	drawShape(squareVao, 6, model, bodyColor);

	// Capul
	model = glm::translate(playerModel, glm::vec3(0.0f, 0.13f, 0.0f));
	model = glm::scale(model, glm::vec3(0.9f, 0.9f, 1.0f));
	drawShape(circleVao, circleSegments * 3, model, glm::vec4(0.95f, 0.8f, 0.65f, 1.0f));

	// Bereta
	model = glm::translate(playerModel, glm::vec3(0.0f, 0.2f, 0.0f));
	model = glm::scale(model, glm::vec3(1.1f, 0.5f, 1.0f));
	drawShape(triangleVao, 3, model, glm::vec4(0.7f, 0.1f, 0.2f, 1.0f));
}

// Lovitura da damage o singura data, la click
void handleSwing(glm::vec2 aimDir, float currentFrame)
{
	if (!swingHitPending)
	{
		return;
	}
	swingHitPending = false;

	for (int i = 0; i < enemyCount; i++)
	{
		if (enemyHealth[i] > 0.0f && isInSwingArc(playerPos, enemyPos[i], aimDir, swingRadius, swingCosHalfAngle))
		{
			// Cate lovituri trebuie: daca pensula e mai slaba, diferenta de putere + 1
			int hitsNeeded = 1;
			if (inkStrength[enemyColorIndex[i]] > inkStrength[brushIndex])
			{
				hitsNeeded = inkStrength[enemyColorIndex[i]] - inkStrength[brushIndex] + 1;
			}
			enemyHealth[i] -= enemyMaxHealth / hitsNeeded;

			// Pata de vopsea pe inamic, in culoarea pensulei
			splatPos[nextSplat] = enemyPos[i];
			splatStart[nextSplat] = currentFrame;
			splatColor[nextSplat] = inkColors[brushIndex];
			splatActive[nextSplat] = true;
			nextSplat = (nextSplat + 1) % splatCount;
		}
	}
}

// Magic Brush, intoarsa spre cursor
void drawBrush(glm::vec2 aimDir, float currentFrame)
{
	// Unghiul directiei spre cursor, in grade
	float aimAngle = acos(aimDir.x) * 180.0f / 3.14159265f;
	if (aimDir.y < 0.0f)
	{
		aimAngle = -aimAngle;
	}

	// In timpul loviturii unghiul merge de la +halfAngle la -halfAngle
	glm::mat4 swingRotation(1.0f);
	if (isSwinging)
	{
		float swingTime = currentFrame - swingStartTime;
		if (swingTime >= swingDuration)
		{
			isSwinging = false;
		}
		else
		{
			float swingAngle = swingHalfAngleDeg - swingTime * 480.0f;
			swingRotation = glm::rotate(glm::mat4(1.0f), swingAngle, glm::vec3(0.0f, 0.0f, 1.0f));
		}
	}

	// Mutam pivotul in mana, apoi rotim
	glm::mat4 brushModel = glm::translate(getPlayerModel(), glm::vec3(0.07f, 0.0f, 0.0f));
	brushModel = glm::rotate(brushModel, aimAngle, glm::vec3(0.0f, 0.0f, 1.0f));
	brushModel = brushModel * swingRotation;

	// Manerul pensulei
	glm::mat4 model = glm::translate(brushModel, glm::vec3(0.08f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.6f, 0.25f, 1.0f));
	drawShape(squareVao, 6, model, glm::vec4(0.5f, 0.3f, 0.1f, 1.0f));

	// Varful, in culoarea aleasa
	model = glm::translate(brushModel, glm::vec3(0.2f, 0.0f, 0.0f));
	model = glm::rotate(model, -90.0f, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.5f, 0.8f, 1.0f));
	drawShape(triangleVao, 3, model, glm::vec4(inkColors[brushIndex], 1.0f));
}

// Paleta care se invarte in jurul jucatorului
void drawPalette()
{
	glm::vec3 dotPositions[3] = {
		glm::vec3(0.02f, 0.02f, 0.0f),
		glm::vec3(-0.025f, 0.01f, 0.0f),
		glm::vec3(0.0f, -0.025f, 0.0f)
	};

	// Prima rotatie o duce pe orbita, translatia da raza, a doua o invarte in jurul ei
	float time = (float)glfwGetTime();
	glm::mat4 paletteModel = glm::rotate(getPlayerModel(), time * 100.0f, glm::vec3(0.0f, 0.0f, 1.0f));
	paletteModel = glm::translate(paletteModel, glm::vec3(0.25f, 0.0f, 0.0f));
	paletteModel = glm::rotate(paletteModel, time * 200.0f, glm::vec3(0.0f, 0.0f, 1.0f));

	// Placa paletei
	glm::mat4 model = glm::scale(paletteModel, glm::vec3(1.0f, 0.75f, 1.0f));
	drawShape(circleVao, circleSegments * 3, model, glm::vec4(0.9f, 0.8f, 0.6f, 1.0f));

	// Cele 3 puncte de culoare
	for (int i = 0; i < 3; i++)
	{
		model = glm::translate(paletteModel, dotPositions[i]);
		model = glm::scale(model, glm::vec3(0.25f, 0.25f, 1.0f));
		drawShape(circleVao, circleSegments * 3, model, glm::vec4(inkColors[i], 1.0f));
	}
}

int main(void)
{
	// Initializare GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		(void)getchar();
		return -1;
	}

#ifdef __APPLE__
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// Fereastra si contextul OpenGL
	window = glfwCreateWindow(windowWidth, windowHeight, "Paint Rush: The Colorless Museum", NULL, NULL);
	if (window == NULL)
	{
		fprintf(stderr, "Failed to open GLFW window.");
		(void)getchar();
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// Initializare GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		(void)getchar();
		glfwTerminate();
		return -1;
	}

	// Viewport-ul cat toata fereastra
	glViewport(0, 0, windowWidth, windowHeight);

	// Culoarea de fundal
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Doua perechi de shadere: simplu si cu textura
	programID = LoadShaders("SimpleVertexShader.vert", "SimpleFragmentShader.frag");
	texProgramID = LoadShaders("TexturedVertexShader.vert", "TexturedFragmentShader.frag");

	// Patratul din lab
	GLfloat vertices[] = {
		0.05f,  0.05f, 0.0f,	// top right
		0.05f, -0.05f, 0.0f,	// bottom right
		-0.05f, -0.05f, 0.0f,	// bottom left
		-0.05f,  0.05f, 0.0f	// top left
	};

	GLuint indices[] = {	// Note that we start from 0
		0, 3, 1,			// first Triangle
		1, 3, 2,			// second Triangle
	};

	// Create VAO, VBO and IBO
	GLuint vbo, ibo;
	glGenVertexArrays(1, &squareVao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);

	// Bind VAO
	glBindVertexArray(squareVao);

	// Bind VBO
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Bind IBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Describe the data in the buffer
	// This will be accesible in the shaders
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), 0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	// Dreptunghiul cu textura: 6 varfuri, desenat cu glDrawArrays
	GLfloat texQuadVertices[] = {
		// pozitie			 // coordonate textura
		0.5f,  0.5f, 0.0f,   0.99f, 0.99f,
		0.5f, -0.5f, 0.0f,   0.99f, 0.01f,
		-0.5f,  0.5f, 0.0f,  0.01f, 0.99f,

		0.5f, -0.5f, 0.0f,   0.99f, 0.01f,
		-0.5f, -0.5f, 0.0f,  0.01f, 0.01f,
		-0.5f,  0.5f, 0.0f,  0.01f, 0.99f
	};

	// VAO si VBO, fara IBO
	GLuint texVbo;
	glGenVertexArrays(1, &texVao);
	glGenBuffers(1, &texVbo);

	glBindVertexArray(texVao);

	glBindBuffer(GL_ARRAY_BUFFER, texVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texQuadVertices), texQuadVertices, GL_STATIC_DRAW);

	// Atributul 0: pozitia varfurilor
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Atributul 1: coordonatele de textura
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Unbind la VAO
	glBindVertexArray(0);

	// Cercul: centrul plus 24 de puncte, din 15 in 15 grade
	GLfloat circleVertices[] = {
		0.0f, 0.0f, 0.0f,
		0.05000f, 0.00000f, 0.0f,
		0.04830f, 0.01294f, 0.0f,
		0.04330f, 0.02500f, 0.0f,
		0.03536f, 0.03536f, 0.0f,
		0.02500f, 0.04330f, 0.0f,
		0.01294f, 0.04830f, 0.0f,
		0.00000f, 0.05000f, 0.0f,
		-0.01294f, 0.04830f, 0.0f,
		-0.02500f, 0.04330f, 0.0f,
		-0.03536f, 0.03536f, 0.0f,
		-0.04330f, 0.02500f, 0.0f,
		-0.04830f, 0.01294f, 0.0f,
		-0.05000f, 0.00000f, 0.0f,
		-0.04830f, -0.01294f, 0.0f,
		-0.04330f, -0.02500f, 0.0f,
		-0.03536f, -0.03536f, 0.0f,
		-0.02500f, -0.04330f, 0.0f,
		-0.01294f, -0.04830f, 0.0f,
		0.00000f, -0.05000f, 0.0f,
		0.01294f, -0.04830f, 0.0f,
		0.02500f, -0.04330f, 0.0f,
		0.03536f, -0.03536f, 0.0f,
		0.04330f, -0.02500f, 0.0f,
		0.04830f, -0.01294f, 0.0f
	};
	// Fiecare triunghi = centrul + doua puncte vecine; ultimul inchide cercul
	GLuint circleIndices[] = {
		0, 1, 2,
		0, 2, 3,
		0, 3, 4,
		0, 4, 5,
		0, 5, 6,
		0, 6, 7,
		0, 7, 8,
		0, 8, 9,
		0, 9, 10,
		0, 10, 11,
		0, 11, 12,
		0, 12, 13,
		0, 13, 14,
		0, 14, 15,
		0, 15, 16,
		0, 16, 17,
		0, 17, 18,
		0, 18, 19,
		0, 19, 20,
		0, 20, 21,
		0, 21, 22,
		0, 22, 23,
		0, 23, 24,
		0, 24, 1
	};
	// VAO, VBO si IBO pentru cerc
	GLuint circleVbo, circleIbo;
	glGenVertexArrays(1, &circleVao);
	glGenBuffers(1, &circleVbo);
	glGenBuffers(1, &circleIbo);

	glBindVertexArray(circleVao);

	glBindBuffer(GL_ARRAY_BUFFER, circleVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(circleVertices), circleVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circleIbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(circleIndices), circleIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), 0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	// Triunghiul (bereta si varful pensulei)
	GLfloat triangleVertices[] = {
		-0.05f, -0.05f, 0.0f,
		0.05f, -0.05f, 0.0f,
		0.0f,  0.05f, 0.0f
	};
	GLuint triangleIndices[] = { 0, 1, 2 };
	// VAO, VBO si IBO pentru triunghi
	GLuint triangleVbo, triangleIbo;
	glGenVertexArrays(1, &triangleVao);
	glGenBuffers(1, &triangleVbo);
	glGenBuffers(1, &triangleIbo);

	glBindVertexArray(triangleVao);

	glBindBuffer(GL_ARRAY_BUFFER, triangleVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangleIndices), triangleIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), 0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	// Pentru fiecare tablou incarcam varianta color si pe cea gri
	for (int i = 0; i < paintingCount; i++)
	{
		paintingTex[i] = LoadImage(paintingFiles[i]);
		paintingGrayTex[i] = LoadImage(paintingGrayFiles[i]);
	}

	// Set a callback for handling mouse cursor position
	glfwSetCursorPosCallback(window, cursor_position_callback);
	// Callback pentru click-ul mouse-ului
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	// Set callback for window resizing
	glfwSetFramebufferSizeCallback(window, window_callback);

	lastFrame = (float)glfwGetTime();

	// Bucla de randare
	while (!glfwWindowShouldClose(window))
	{
		// deltaTime = timpul trecut de la cadrul anterior
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Double buffering: schimbam bufferele
		glfwSwapBuffers(window);

		// Evenimente, input si schimbarea scenei
		glfwPollEvents();
		processInput(window);

		// Proiectie ortografica; x merge de la -aspect la aspect ca sa nu se deformeze imaginea
		projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);

		// Curatarea ecranului
		glClear(GL_COLOR_BUFFER_BIT);

		bool playerHit = false;

		// Fundalul (galeria sau tabloul curent)
		if (gameState == STATE_GALLERY)
		{
			drawGallery();
		}
		else
		{
			drawTextured(paintingGrayTex[currentLevel], paintingTex[currentLevel], glm::vec2(0.0f, 0.0f), glm::vec2(2.0f * aspect, 2.0f), paintingColorAmount, 1.0f);
		}

		// Restul se deseneaza cu shaderul simplu
		glUseProgram(programID);

		// Doar in tablou: inamici, progres si pete
		if (gameState == STATE_PAINTING)
		{
			playerHit = updateEnemies();
			drawEnemies();
			updateLevel(currentFrame);
			drawSplats(currentFrame);
		}

		glm::vec2 aimDir = getAimDir();

		// Jucatorul, lovitura, pensula si paleta
		drawPlayer(playerHit);
		handleSwing(aimDir, currentFrame);
		drawBrush(aimDir, currentFrame);
		drawPalette();
	}

	// Stergerea resurselor
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glDeleteVertexArrays(1, &squareVao);
	glDeleteBuffers(1, &texVbo);
	glDeleteVertexArrays(1, &texVao);
	glDeleteBuffers(1, &circleVbo);
	glDeleteBuffers(1, &circleIbo);
	glDeleteVertexArrays(1, &circleVao);
	glDeleteBuffers(1, &triangleVbo);
	glDeleteBuffers(1, &triangleIbo);
	glDeleteVertexArrays(1, &triangleVao);
	glDeleteProgram(programID);
	glDeleteProgram(texProgramID);

	// Oprim GLFW
	glfwTerminate();

	return 0;
}
