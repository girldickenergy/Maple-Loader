#pragma once

#include <string>

class Game
{
	unsigned int id;
	std::string name;
	void* iconTexture;
	void* bannerTexture;
public:
	Game(unsigned int id, const std::string& name);
	Game() = default;
	~Game();

	unsigned int GetID();
	const std::string& GetName();
	void* GetIconTexture();
	void* GetBannerTexture();
};
