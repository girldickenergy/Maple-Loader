#pragma once

#include <vector>

#include "Game.h"
#include "Cheat.h"

struct User
{
	char Username[24];
	char Password[256];
	std::string SessionID;
	std::string DiscordID;
	std::string AvatarHash;
	std::vector<Game*> Games;
	std::vector<Cheat*> Cheats;
	Game* CurrentGame = nullptr;
	Cheat* CurrentCheat = nullptr;

	void ResetSensitiveFields()
	{
		memset(Password, 0, sizeof(Password));
	}
};
