#version 330 core

// Coordonatele de textura din vertex shader
in vec2 texCoords;
// Culoarea finala
out vec4 fragColor;

// Textura gri si textura color
uniform sampler2D texture1;
uniform sampler2D texture2;
// Cat din textura color se vede (0 = gri, 1 = color)
uniform float colorAmount;
uniform float brightness;

void main()
{
	// Culoarea din ambele texturi
	vec4 color1 = texture(texture1, texCoords);
	vec4 color2 = texture(texture2, texCoords);
	// Amestecul celor doua texturi
	fragColor = mix(color1, color2, colorAmount) * brightness;
}
