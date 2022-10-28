#pragma once
#include <string>
#include <vector>

#include "../../Cheat.h"
#include "../../Game.h"

enum class LoginResult : unsigned int
{
	Success = 0x0,
	IncorrectCredentials = 0x1,
	VersionMismatch = 0x2,
	HWIDMismatch = 0x3,
	Banned = 0x4,
	UnknownError = 0x5
};

class LoginResponse
{
	LoginResult result;
	std::string sessionToken;
	std::string discordID;
	std::string discordAvatarHash;
	std::vector<Game*> games;
	std::vector<Cheat*> cheats;

	LoginResponse(LoginResult result, const std::string& sessionToken, const std::string& discordID, const std::string& discordAvatarHash, const std::vector<Game*>& games, const std::vector<Cheat*>& cheats);
public:
	LoginResult GetResult();
	const std::string& GetSessionToken();
	const std::string& GetDiscordID();
	const std::string& GetDiscordAvatarHash();
	const std::vector<Game*>& GetGames();
	const std::vector<Cheat*>& GetCheats();

	static LoginResponse Deserialize(const std::vector<unsigned char>& payload);
};