#include "TextureLoader.h"

#include <glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <WinSock2.h>
#include <curl.h>

#include <vector>

#include "../Strings/StringUtilities.h"

void* TextureLoader::loadTextureInternal(unsigned char* data, int width, int height)
{
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	return (void*)tex;
}

void* TextureLoader::LoadTextureFromMemory(const unsigned char* data, int size)
{
	int width, height, n;
	unsigned char* textureData = stbi_load_from_memory(data, size, &width, &height, &n, 4);
	if (!data)
		return nullptr;

	return loadTextureInternal(textureData, width, height);
}

void* TextureLoader::LoadTextureFromURL(const std::string& url)
{
	if (CURL* curl = curl_easy_init())
	{
		std::string readBuffer;

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		CURLcode res = curl_easy_perform(curl);

		if (res == CURLE_OK)
		{
			const std::vector<unsigned char> bytes = StringUtilities::StringToByteArray(readBuffer);
			return LoadTextureFromMemory(bytes.data(), bytes.size());
		}
	}

	return nullptr;
}


void TextureLoader::FreeTexture(void* textureId)
{
	GLuint texID = (GLuint)textureId;
	glDeleteTextures(1, &texID);
}
