#pragma once

#include <string>

class Game
{
	uint32_t m_ID;
	std::string m_Name;
	void* m_IconTexture;
	void* m_BannerTexture;
public:
	Game() = default;
	~Game();

	uint32_t GetID();
	const std::string& GetName();
	void* GetIconTexture();
	void* GetBannerTexture();

	static void Register();
};
