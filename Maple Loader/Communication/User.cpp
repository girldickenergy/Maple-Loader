#include "User.h"

#include "../Utilities/Textures/TextureLoader.h"
#include "../Utilities/Security/xorstr.hpp"
#include "../UI/Assets/Textures.h"

User::User(const std::string& username, const std::string& sessionToken, const std::string& discordID, const std::string& avatarHash)
{
	this->username = username;
	this->sessionToken = sessionToken;
	this->discordID = discordID;
	this->avatarHash = avatarHash;
}

User::~User()
{
	if (avatarTexture)
		TextureLoader::FreeTexture(avatarTexture);
}

const std::string& User::GetUsername()
{
	return username;
}

const std::string& User::GetSessionToken()
{
	return sessionToken;
}

const std::string& User::GetDiscordID()
{
	return discordID;
}

const std::string& User::GetAvatarHash()
{
	return avatarHash;
}

void* User::GetAvatarTexture()
{
	if (!avatarTexture)
	{
		avatarTexture = TextureLoader::LoadTextureFromURL(xorstr_("https://cdn.discordapp.com/avatars/") + discordID + xorstr_("/") + avatarHash + xorstr_(".png?size=64"));
		if (!avatarTexture)
			avatarTexture = TextureLoader::LoadTextureFromMemory(Textures::DefaultAvatar, Textures::DefaultAvatarSize);
	}

	return avatarTexture;
}
