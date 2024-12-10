#pragma once

#include <string>
#include <vector>

#include "../IPacket.h"
#include "../RequestResult.h"
#include "../../Cheat.h"
#include "../../Game.h"

class LoginResponse : IPacket
{
	int32_t m_Result;
	std::string m_SessionToken;
	std::string m_DiscordID;
	std::string m_DiscordAvatarHash;
	std::vector<Game> m_Games;
	std::vector<Cheat> m_Cheats;

	static uint32_t GetStaticIdentifier();
public:
	RequestResult GetResult();
	const std::string& GetSessionToken();
	const std::string& GetDiscordID();
	const std::string& GetDiscordAvatarHash();
	const std::vector<Game>& GetGames();
	const std::vector<Cheat>& GetCheats();

	uint32_t GetIdentifier() override;
	static void Register();
};