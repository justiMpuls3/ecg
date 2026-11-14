#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "glew/glew.h"
#include "glfw/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shaders/Utils/shaderLoader.hpp"

#include "Globals/globals.hpp"

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	cursorPos.x = (2.0f * (float)xpos / (float)windowWidth - 1.0f) * aspect;
	cursorPos.y = 1.0f - 2.0f * (float)ypos / (float)windowHeight;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !isSwinging)
	{
		isSwinging = true;
		swingStartTime = (float)glfwGetTime();
		swingHitPending = true;
	}
}

void window_callback(GLFWwindow* window, int new_width, int new_height)
{
	if (new_height == 0)
	{
		return;
	}
	windowWidth = new_width;
	windowHeight = new_height;
	aspect = (float)new_width / (float)new_height;
	glViewport(0, 0, new_width, new_height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) brushColor = glm::vec3(0.2f, 0.4f, 1.0f);
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) brushColor = glm::vec3(1.0f, 0.9f, 0.1f);
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) brushColor = glm::vec3(0.85f, 0.65f, 0.13f);

	glm::vec2 moveDir(0.0f, 0.0f);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir.y += 1.0f;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir.y -= 1.0f;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir.x += 1.0f;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir.x -= 1.0f;
	if (glm::length(moveDir) > 0.0f)
	{
		playerPos += glm::normalize(moveDir) * playerSpeed * deltaTime;
	}

	playerPos.x = glm::clamp(playerPos.x, -aspect + 0.05f, aspect - 0.05f);
	playerPos.y = glm::clamp(playerPos.y, -0.95f, 0.95f);
}

void createShape(GLuint& vao, GLuint& vbo, GLuint& ibo, const GLfloat* vertices, GLsizeiptr verticesSize, const GLuint* indices, GLsizeiptr indicesSize)
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), 0);
	glEnableVertexAttribArray(0);
}

void drawShape(GLuint programID, GLuint vao, int indexCount, const glm::mat4& model, const glm::vec4& color)
{
	glm::mat4 projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);
	glm::mat4 matrix = projection * model;

	glUniformMatrix4fv(glGetUniformLocation(programID, "transform"), 1, GL_FALSE, glm::value_ptr(matrix));
	glUniform4fv(glGetUniformLocation(programID, "color"), 1, glm::value_ptr(color));

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

float colorMatchDamage(const glm::vec3& cp, const glm::vec3& cw, float damageBase)
{
	float d = glm::length(cp - cw) / sqrt(3.0f);
	return damageBase * (1.0f - d);
}

bool isInSwingArc(const glm::vec2& pPlayer, const glm::vec2& pEnemy, const glm::vec2& dir, float R, float halfAngle)
{
	glm::vec2 toEnemy = pEnemy - pPlayer;
	float dist = glm::length(toEnemy);
	if (dist > R)
	{
		return false;
	}
	if (dist < 0.0001f)
	{
		return true;
	}
	return glm::dot(dir, glm::normalize(toEnemy)) >= cos(halfAngle);
}

int main(void)
{
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

	window = glfwCreateWindow(windowWidth, windowHeight, "Paint Rush: The Colorless Museum", NULL, NULL);
	if (window == NULL)
	{
		fprintf(stderr, "Failed to open GLFW window.");
		(void)getchar();
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		(void)getchar();
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, windowWidth, windowHeight);

	glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	GLuint programID = LoadShaders("SimpleVertexShader.vert", "SimpleFragmentShader.frag");

	GLfloat vertices[] = {
		0.05f,  0.05f, 0.0f,
		0.05f, -0.05f, 0.0f,
		-0.05f, -0.05f, 0.0f,
		-0.05f,  0.05f, 0.0f
	};

	GLuint indices[] = {
		0, 3, 1,
		1, 3, 2,
	};

	GLuint vao, vbo, ibo;
	createShape(vao, vbo, ibo, vertices, sizeof(vertices), indices, sizeof(indices));

	const int circleSegments = 32;
	GLfloat circleVertices[(circleSegments + 1) * 3];
	GLuint circleIndices[circleSegments * 3];
	circleVertices[0] = 0.0f;
	circleVertices[1] = 0.0f;
	circleVertices[2] = 0.0f;
	for (int i = 0; i < circleSegments; i++)
	{
		float angle = 2.0f * 3.14159265f * i / circleSegments;
		circleVertices[(i + 1) * 3] = 0.05f * cos(angle);
		circleVertices[(i + 1) * 3 + 1] = 0.05f * sin(angle);
		circleVertices[(i + 1) * 3 + 2] = 0.0f;
		circleIndices[i * 3] = 0;
		circleIndices[i * 3 + 1] = i + 1;
		circleIndices[i * 3 + 2] = (i + 1) % circleSegments + 1;
	}
	GLuint circleVao, circleVbo, circleIbo;
	createShape(circleVao, circleVbo, circleIbo, circleVertices, sizeof(circleVertices), circleIndices, sizeof(circleIndices));

	GLfloat triangleVertices[] = {
		-0.05f, -0.05f, 0.0f,
		0.05f, -0.05f, 0.0f,
		0.0f,  0.05f, 0.0f
	};
	GLuint triangleIndices[] = { 0, 1, 2 };
	GLuint triangleVao, triangleVbo, triangleIbo;
	createShape(triangleVao, triangleVbo, triangleIbo, triangleVertices, sizeof(triangleVertices), triangleIndices, sizeof(triangleIndices));

	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetFramebufferSizeCallback(window, window_callback);

	const int enemyCount = 3;
	glm::vec2 enemyPos[enemyCount] = {
		glm::vec2(-1.0f, 0.7f),
		glm::vec2(1.0f, -0.6f),
		glm::vec2(-0.9f, -0.7f)
	};
	float enemyHealth[enemyCount] = { 100.0f, 100.0f, 100.0f };
	float enemyMaxHealth[enemyCount] = { 100.0f, 100.0f, 100.0f };
	float enemySize[enemyCount] = { 1.0f, 1.0f, 1.0f };
	float enemySpeed[enemyCount] = { 0.25f, 0.25f, 0.25f };
	glm::vec3 enemyWeakColor[enemyCount] = {
		glm::vec3(0.2f, 0.4f, 1.0f),
		glm::vec3(1.0f, 0.9f, 0.1f),
		glm::vec3(0.85f, 0.65f, 0.13f)
	};

	glm::vec3 blobPositions[4] = {
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.05f, 0.04f, 0.0f),
		glm::vec3(-0.06f, 0.03f, 0.0f),
		glm::vec3(0.0f, -0.07f, 0.0f)
	};
	float blobScales[4] = { 1.6f, 0.9f, 0.8f, 0.7f };

	const int splatCount = 8;
	const float splatDuration = 0.6f;
	glm::vec2 splatPos[splatCount];
	float splatStart[splatCount];
	glm::vec3 splatColor[splatCount];
	bool splatActive[splatCount] = {};
	int nextSplat = 0;

	glm::vec3 splatPartPositions[4] = {
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.06f, 0.05f, 0.0f),
		glm::vec3(-0.07f, 0.02f, 0.0f),
		glm::vec3(0.01f, -0.07f, 0.0f)
	};
	float splatPartScales[4] = { 1.0f, 0.5f, 0.4f, 0.45f };

	lastFrame = (float)glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwSwapBuffers(window);
		glfwPollEvents();
		processInput(window);

		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(programID);

		glm::mat4 model;
		bool playerHit = false;

		for (int i = 0; i < enemyCount; i++)
		{
			if (enemyHealth[i] <= 0.0f)
			{
				continue;
			}

			float enemyRadius = 0.09f * enemySize[i];
			glm::vec2 toPlayer = playerPos - enemyPos[i];
			glm::vec2 chaseDir(0.0f, 0.0f);
			if (glm::length(toPlayer) > 0.0001f)
			{
				chaseDir = glm::normalize(toPlayer);
			}
			enemyPos[i] += chaseDir * enemySpeed[i] * deltaTime;

			if (glm::length(playerPos - enemyPos[i]) < enemyRadius + playerRadius)
			{
				playerHit = true;
				playerHealth -= 20.0f * deltaTime;
				enemyPos[i] = playerPos - chaseDir * (enemyRadius + playerRadius);
			}

			glm::mat4 enemyModel = glm::translate(glm::mat4(1.0f), glm::vec3(enemyPos[i], 0.0f));
			enemyModel = glm::scale(enemyModel, glm::vec3(enemySize[i], enemySize[i], 1.0f));

			for (int j = 0; j < 4; j++)
			{
				model = glm::translate(enemyModel, blobPositions[j]);
				model = glm::scale(model, glm::vec3(blobScales[j], blobScales[j], 1.0f));
				drawShape(programID, circleVao, circleSegments * 3, model, glm::vec4(0.05f, 0.05f, 0.08f, 1.0f));
			}

			model = glm::scale(enemyModel, glm::vec3(0.5f, 0.5f, 1.0f));
			drawShape(programID, circleVao, circleSegments * 3, model, glm::vec4(enemyWeakColor[i], 1.0f));
		}

		if (playerHealth <= 0.0f)
		{
			playerHealth = playerMaxHealth;
			playerPos = glm::vec2(0.0f, 0.0f);
		}

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
			float splatScale = glm::mix(0.6f, 2.2f, splatProgress);
			glm::vec4 color = glm::vec4(glm::mix(splatColor[i], backgroundColor, splatProgress), 1.0f);

			glm::mat4 splatModel = glm::translate(glm::mat4(1.0f), glm::vec3(splatPos[i], 0.0f));
			splatModel = glm::scale(splatModel, glm::vec3(splatScale, splatScale, 1.0f));

			for (int j = 0; j < 4; j++)
			{
				model = glm::translate(splatModel, splatPartPositions[j]);
				model = glm::scale(model, glm::vec3(splatPartScales[j], splatPartScales[j], 1.0f));
				drawShape(programID, circleVao, circleSegments * 3, model, color);
			}
		}

		glm::mat4 playerModel = glm::translate(glm::mat4(1.0f), glm::vec3(playerPos, 0.0f));

		model = glm::scale(playerModel, glm::vec3(1.4f, 1.8f, 1.0f));
		if (playerHit)
		{
			drawShape(programID, vao, 6, model, glm::vec4(0.9f, 0.2f, 0.2f, 1.0f));
		}
		else
		{
			drawShape(programID, vao, 6, model, glm::vec4(0.2f, 0.5f, 0.6f, 1.0f));
		}

		model = glm::translate(playerModel, glm::vec3(0.0f, 0.13f, 0.0f));
		model = glm::scale(model, glm::vec3(0.9f, 0.9f, 1.0f));
		drawShape(programID, circleVao, circleSegments * 3, model, glm::vec4(0.95f, 0.8f, 0.65f, 1.0f));

		model = glm::translate(playerModel, glm::vec3(0.0f, 0.2f, 0.0f));
		model = glm::scale(model, glm::vec3(1.1f, 0.5f, 1.0f));
		drawShape(programID, triangleVao, 3, model, glm::vec4(0.7f, 0.1f, 0.2f, 1.0f));

		glm::vec2 aimDir = cursorPos - playerPos;
		if (glm::length(aimDir) > 0.0001f)
		{
			aimDir = glm::normalize(aimDir);
		}
		else
		{
			aimDir = glm::vec2(1.0f, 0.0f);
		}

		if (swingHitPending)
		{
			swingHitPending = false;
			for (int i = 0; i < enemyCount; i++)
			{
				if (enemyHealth[i] > 0.0f && isInSwingArc(playerPos, enemyPos[i], aimDir, swingRadius, swingHalfAngle))
				{
					enemyHealth[i] -= colorMatchDamage(brushColor, enemyWeakColor[i], damageBase);
					enemySize[i] = glm::max(enemyHealth[i], 0.0f) / enemyMaxHealth[i];
					splatPos[nextSplat] = enemyPos[i];
					splatStart[nextSplat] = currentFrame;
					splatColor[nextSplat] = brushColor;
					splatActive[nextSplat] = true;
					nextSplat = (nextSplat + 1) % splatCount;
				}
			}
		}

		glm::mat4 aimRotation(1.0f);
		aimRotation[0][0] = aimDir.x;
		aimRotation[0][1] = aimDir.y;
		aimRotation[1][0] = -aimDir.y;
		aimRotation[1][1] = aimDir.x;

		glm::mat4 swingRotation(1.0f);
		if (isSwinging)
		{
			float swingProgress = (currentFrame - swingStartTime) / swingDuration;
			if (swingProgress >= 1.0f)
			{
				isSwinging = false;
			}
			else
			{
				float swingAngle = glm::mix(swingHalfAngle, -swingHalfAngle, swingProgress);
				swingRotation = glm::rotate(glm::mat4(1.0f), glm::degrees(swingAngle), glm::vec3(0.0f, 0.0f, 1.0f));
			}
		}

		glm::mat4 brushModel = glm::translate(playerModel, glm::vec3(0.07f, 0.0f, 0.0f));
		brushModel = brushModel * aimRotation * swingRotation;

		model = glm::translate(brushModel, glm::vec3(0.08f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.6f, 0.25f, 1.0f));
		drawShape(programID, vao, 6, model, glm::vec4(0.5f, 0.3f, 0.1f, 1.0f));

		model = glm::translate(brushModel, glm::vec3(0.2f, 0.0f, 0.0f));
		model = glm::rotate(model, -90.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.8f, 1.0f));
		drawShape(programID, triangleVao, 3, model, glm::vec4(brushColor, 1.0f));

		float time = (float)glfwGetTime();
		glm::mat4 paletteModel = glm::rotate(playerModel, glm::degrees(time * 2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		paletteModel = glm::translate(paletteModel, glm::vec3(0.25f, 0.0f, 0.0f));
		paletteModel = glm::rotate(paletteModel, glm::degrees(time * 4.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		model = glm::scale(paletteModel, glm::vec3(1.0f, 0.75f, 1.0f));
		drawShape(programID, circleVao, circleSegments * 3, model, glm::vec4(0.9f, 0.8f, 0.6f, 1.0f));

		model = glm::translate(paletteModel, glm::vec3(0.02f, 0.02f, 0.0f));
		model = glm::scale(model, glm::vec3(0.25f, 0.25f, 1.0f));
		drawShape(programID, circleVao, circleSegments * 3, model, glm::vec4(0.2f, 0.4f, 1.0f, 1.0f));

		model = glm::translate(paletteModel, glm::vec3(-0.025f, 0.01f, 0.0f));
		model = glm::scale(model, glm::vec3(0.25f, 0.25f, 1.0f));
		drawShape(programID, circleVao, circleSegments * 3, model, glm::vec4(1.0f, 0.9f, 0.1f, 1.0f));

		model = glm::translate(paletteModel, glm::vec3(0.0f, -0.025f, 0.0f));
		model = glm::scale(model, glm::vec3(0.25f, 0.25f, 1.0f));
		drawShape(programID, circleVao, circleSegments * 3, model, glm::vec4(0.85f, 0.65f, 0.13f, 1.0f));
	}

	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &circleVbo);
	glDeleteBuffers(1, &circleIbo);
	glDeleteVertexArrays(1, &circleVao);
	glDeleteBuffers(1, &triangleVbo);
	glDeleteBuffers(1, &triangleIbo);
	glDeleteVertexArrays(1, &triangleVao);
	glDeleteProgram(programID);

	glfwTerminate();

	return 0;
}
