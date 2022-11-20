#include "LoginResponse.h"

#include "json.hpp"
#include "ThemidaSDK.h"

#include "../../../Utilities/Strings/StringUtilities.h"
#include "../../../Utilities/Security/xorstr.hpp"
#include "../../Crypto/CryptoProvider.h"

LoginResponse::LoginResponse(LoginResult result, const std::string& sessionToken, const std::string& discordID, const std::string& discordAvatarHash, const std::vector<Game*>& games, const std::vector<Cheat*>& cheats)
{
	this->result = result;
	this->sessionToken = sessionToken;
	this->discordID = discordID;
	this->discordAvatarHash = discordAvatarHash;
	this->games = games;
	this->cheats = cheats;
}

LoginResult LoginResponse::GetResult()
{
	return result;
}

const std::string& LoginResponse::GetSessionToken()
{
	return sessionToken;
}

const std::string& LoginResponse::GetDiscordID()
{
	return discordID;
}

const std::string& LoginResponse::GetDiscordAvatarHash()
{
	return discordAvatarHash;
}

const std::vector<Game*>& LoginResponse::GetGames()
{
	return games;
}

const std::vector<Cheat*>& LoginResponse::GetCheats()
{
	return cheats;
}

#pragma optimize("", off)
LoginResponse LoginResponse::Deserialize(const std::vector<unsigned char>& payload)
{
	VM_SHARK_BLACK_START
	STR_ENCRYPT_START

	nlohmann::json jsonPayload = nlohmann::json::parse(StringUtilities::ByteArrayToString(CryptoProvider::GetInstance()->AESDecrypt(payload)));
	
	LoginResult result = jsonPayload[xorstr_("Result")];
	LoginResponse response = LoginResponse(result, "", "", "", {}, {});
	if (result == LoginResult::Success)
	{
		std::string sessionToken = jsonPayload[xorstr_("SessionToken")];
		std::string discordID = jsonPayload[xorstr_("DiscordID")];
		std::string discordAvatarHash = jsonPayload[xorstr_("DiscordAvatarHash")];

		std::vector<Game*> games;
		for (auto game : jsonPayload[xorstr_("Games")])
		{
			unsigned int id = game[xorstr_("ID")];
			std::string name = game[xorstr_("Name")];

			games.push_back(new Game(id, name));
		}

		std::vector<Cheat*> cheats;
		for (auto cheat : jsonPayload[xorstr_("Cheats")])
		{
			unsigned int id = cheat[xorstr_("ID")];
			unsigned int gameID = cheat[xorstr_("GameID")];
			std::string name = cheat[xorstr_("Name")];

			std::vector<std::string> releaseStreams;
			for (std::string releaseStream : cheat[xorstr_("ReleaseStreams")])
				releaseStreams.push_back(releaseStream);

			unsigned int startingPrice = cheat[xorstr_("StartingPrice")];
			CheatStatus status = cheat[xorstr_("Status")];
			std::string expiresOn = cheat[xorstr_("ExpiresOn")];

			cheats.push_back(new Cheat(id, gameID, name, releaseStreams, startingPrice, status, expiresOn));
		}

		response = LoginResponse(result, sessionToken, discordID, discordAvatarHash, games, cheats);
	}

	STR_ENCRYPT_END
	VM_SHARK_BLACK_END

	return response;
}
#pragma optimize("", on)
