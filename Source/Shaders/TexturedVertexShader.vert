#version 330 core

// Pozitia varfului (location 0) si coordonatele de textura (location 1)
layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec2 textureCoords;

// Matricea de transformare
uniform mat4 transform;

// Coordonatele de textura trimise la fragment shader
out vec2 texCoords;

void main()
{
	// Se aplica transformarea pe pozitia varfului
	gl_Position = transform * vec4(vertexPos, 1.0);
	texCoords = textureCoords;
}
