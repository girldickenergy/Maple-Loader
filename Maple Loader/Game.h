#pragma once

#include <d3d9.h>

#include "Utils/TextureUtils.h"

#include "../AntiDebug/xorstr.hpp"

struct Game
{
	int ID;
	std::string Name;
	std::string ModuleName;
	IDirect3DTexture9* Icon;
	IDirect3DTexture9* Banner;

	Game(int id, std::string name, std::string moduleName)
	{
		ID = id;
		Name = name;
		ModuleName = moduleName;

		Icon = TextureUtils::CreateTextureFromURL(xor ("https://maple.software/assets/games/icons/") + std::to_string(ID) + xor (".png"));
		Banner = TextureUtils::CreateTextureFromURL(xor ("https://maple.software/assets/games/banners/") + std::to_string(ID) + xor (".png"));
	}
};
