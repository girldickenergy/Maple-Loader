#pragma once

#include <WinSock2.h>
#include <curl.h>

#include "StringUtilities.h"
#include "../UI/UI.h"

#include "../UI/DirectX/d3dx9tex.h"
#pragma comment(lib, "D3dx9")

class TextureUtils
{
	static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp)
	{
		static_cast<std::string*>(userp)->append(static_cast<char*>(contents), size * nmemb);
		return size * nmemb;
	}
public:
	static IDirect3DTexture9* CreateTexture(LPCVOID data, int size)
	{
		IDirect3DTexture9* ret;
		HRESULT hr = D3DXCreateTextureFromFileInMemory(UI::D3DDevice, data, size, &ret);

		return hr == S_OK ? ret : nullptr;
	}

	static IDirect3DTexture9* CreateTextureFromURL(std::string url)
	{
		CURL* curl = curl_easy_init();
		if (curl)
		{
			std::string readBuffer;

			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
			CURLcode res = curl_easy_perform(curl);
			if (res == CURLE_OK)
			{
				std::vector<unsigned char> bytes = StringUtilities::StringToByteArray(readBuffer);
				return CreateTexture(bytes.data(), bytes.size());
			}
		}

		return nullptr;
	}
};