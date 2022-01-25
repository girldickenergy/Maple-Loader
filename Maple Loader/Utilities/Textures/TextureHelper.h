#pragma once

#include <d3d9.h>
#include <string>

class TextureHelper
{
	static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp)
	{
		static_cast<std::string*>(userp)->append(static_cast<char*>(contents), size * nmemb);
		return size * nmemb;
	}
public:
	static IDirect3DTexture9* CreateTexture(LPCVOID data, int size);
	static IDirect3DTexture9* CreateTextureFromURL(const std::string& url);
};