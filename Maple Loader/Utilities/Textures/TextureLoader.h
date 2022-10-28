#pragma once

#include <string>

class TextureLoader
{
	static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp)
	{
		static_cast<std::string*>(userp)->append(static_cast<char*>(contents), size * nmemb);
		return size * nmemb;
	}

	static void* loadTextureInternal(unsigned char* data, int width, int height);
public:
	static void* LoadTextureFromMemory(const unsigned char* data, int size);
	static void* LoadTextureFromURL(const std::string& url);
	static void FreeTexture(void* textureId);
};
