#pragma once

#include <string>

class User
{
	std::string username;
	std::string sessionToken;
	std::string discordID;
	std::string avatarHash;
	void* avatarTexture;
public:
	User(const std::string& username, const std::string& sessionToken, const std::string& discordID, const std::string& avatarHash);
	User() = default;
	~User();

	const std::string& GetUsername();
	const std::string& GetSessionToken();
	const std::string& GetDiscordID();
	const std::string& GetAvatarHash();
	void* GetAvatarTexture();
};