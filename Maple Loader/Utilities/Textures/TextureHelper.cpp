#include "TextureHelper.h"

#include <d3dx9tex.h>
#pragma comment(lib, "D3dx9")

#include <WinSock2.h>
#include <curl.h>

#include "../Strings/StringUtilities.h"
#include "../../UI/UI.h"

IDirect3DTexture9* TextureHelper::CreateTexture(LPCVOID data, int size)
{
	IDirect3DTexture9* ret;
	HRESULT hr = D3DXCreateTextureFromFileInMemory(UI::D3DDevice, data, size, &ret);

	return hr == S_OK ? ret : nullptr;
}

IDirect3DTexture9* TextureHelper::CreateTextureFromURL(const std::string& url)
{
	CURL* curl = curl_easy_init();
	if (curl)
	{
		std::string readBuffer;

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
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