#pragma once
#include "texture.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

// Include global variables
#include "../Globals/globals.hpp"

namespace ImageUtils
{
	struct ImageData
	{
		std::vector<unsigned char> data;
		unsigned int width = 0;
		unsigned int height = 0;
		unsigned int imageSize = 0;
		std::string format = "UNKNOWN";
	};

	void gracefulExit()
	{
		if (window)
		{
			glfwHideWindow(window);
		}
		(void)getchar();
		std::exit(EXIT_FAILURE);
	}

	namespace ImageLoadingUtils
	{
		ImageData* loadBMP(const char* imageFilePath)
		{
			const unsigned int bmpHeaderSize = 54;
			unsigned char header[bmpHeaderSize];
			unsigned int dataPos;
			unsigned int imageSize;
			unsigned int width, height;

			FILE* file = nullptr;
			errno_t err = fopen_s(&file, imageFilePath, "rb");
			if (err || !file)
			{
				std::cerr << imageFilePath << " could not be opened for some reason. Error number: " << err << ".\n";
				ImageUtils::gracefulExit();
			}

			// Checking if the file is actually a BMP image

			// That means the file content is a 54 byte header followed by the actual image
			// So we use the fread function to read the first 54 bytes of the file into the header array
			if (fread(header, 1, bmpHeaderSize, file) != bmpHeaderSize)
			{
				std::cerr << "Not a correct BMP file.\n";
				ImageUtils::gracefulExit();
			}

			// The first two bytes should be the characters B and M
			// And other parts of the header should contain the BMP format magic numbers
			if (header[0] != 'B' || header[1] != 'M' ||
				*(int*)&(header[0x1C]) != 24 || *(int*)&(header[0x1E]) != 0)
			{
				std::cerr << "Not a correct BMP file.\n";
				ImageUtils::gracefulExit();
			}

			// We also find some useful information in the header section about the actual image
			dataPos = *(int*)&(header[0x0A]);
			imageSize = *(int*)&(header[0x22]);
			width = *(int*)&(header[0x12]);
			height = *(int*)&(header[0x16]);

			// Some failsafes
			if (imageSize == 0)
			{
				imageSize = width * height * 3;
			}
			if (dataPos == 0)
			{
				dataPos = 54;
			}

			// Now we just prepare the ImageData structure for return
			ImageData* imageData = new ImageData;
			// We resize the data array to able to hold the correct data size
			imageData->data.resize(imageSize);
			// Because we have already started reading from the file once, we can simply call 
			// fread again with the imageSize as the count parameter, since the image content is the only thing left in the file
			fread(imageData->data.data(), 1, imageSize, file);
			// Now, because we have finished parsing the file, we should close our program's access to it
			fclose(file);

			imageData->width = width;
			imageData->height = height;
			imageData->imageSize = imageSize;
			imageData->format = "BMP";

			return imageData;
		}
	}

	constexpr unsigned int str2intHasher(const char* str, int h = 0)
	{
		// Some random function to hash strings to integers at compile time for switch-case statements
		return !str[h] ? 5381 : (str2intHasher(str, h + 1) * 33) ^ str[h];
	}

	ImageData* loadImageData(const char* imageFilePath)
	{
		const char* imageFormat = nullptr;
		size_t dotPosition = std::string(imageFilePath).find_last_of('.');
		if (dotPosition != std::string::npos)
		{
			imageFormat = imageFilePath + dotPosition + 1;
		}
		else
		{
			std::cerr << "Cannot determine the image format for file: " << imageFilePath << ".\n";
			ImageUtils::gracefulExit();
		}

		std::cout << "Reading image from: " << imageFilePath << ".\n";

		switch (str2intHasher(imageFormat))
		{
		case str2intHasher("bmp"):
		case str2intHasher("BMP"):
			return ImageLoadingUtils::loadBMP(imageFilePath);
			break;
		default:
			std::cerr << "Unsupported image format: " << imageFormat << "!\n";
			std::cerr << "Implement your own parsing function for it!\n";
			ImageUtils::gracefulExit();
			break;
		}

		return nullptr;
	}

	GLuint CreateOpenGLTexture(ImageData* imageData)
	{
		// Now that we have the image data in memory,
		// We can generate a texture object and store its ID
		GLuint textureID;
		glGenTextures(1, &textureID);

		// Bind the generated ID to the 2D texture target
		glBindTexture(GL_TEXTURE_2D, textureID);
		// Upload the image data to the currently bound 2D texture
		glTexImage2D(GL_TEXTURE_2D,
			0, GL_RGB, imageData->width, imageData->height, 0,
			GL_BGR, GL_UNSIGNED_BYTE, imageData->data.data());

		// Set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

		// Set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glGenerateMipmap(GL_TEXTURE_2D); // Optional, if we need mipmaps

		// We can now safely delete the image data from memory
		delete imageData;

		// Return the ID of the texture
		return textureID;
	}
}
[[nodiscard]]
GLuint LoadImage(const char* imageFilePath)
{
	// Parse image file into ImageData structure
	ImageUtils::ImageData* imageData = ImageUtils::loadImageData(imageFilePath);

	// Create OpenGL texture from abovementioned structure
	GLuint textureID = ImageUtils::CreateOpenGLTexture(imageData);

	return textureID;
}