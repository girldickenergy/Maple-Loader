#include "LoginResponse.h"

#include "entt.hpp"
#include "Fnv1a.h"

#include "../PacketRegistrar.h"

static const PacketRegistrar<LoginResponse> registrar;

uint32_t LoginResponse::GetStaticIdentifier()
{
	return Hash32Fnv1aConst("LoginResponse");
}

RequestResult LoginResponse::GetResult()
{
	return static_cast<RequestResult>(m_Result);
}

const std::string& LoginResponse::GetSessionToken()
{
	return m_SessionToken;
}

const std::string& LoginResponse::GetDiscordID()
{
	return m_DiscordID;
}

const std::string& LoginResponse::GetDiscordAvatarHash()
{
	return m_DiscordAvatarHash;
}

const std::vector<Game>& LoginResponse::GetGames()
{
	return m_Games;
}

const std::vector<Cheat>& LoginResponse::GetCheats()
{
	return m_Cheats;
}

uint32_t LoginResponse::GetIdentifier()
{
	return GetStaticIdentifier();
}

void LoginResponse::Register()
{
	if (registrar.IsRegistered)
		return;

	entt::meta<LoginResponse>().type(GetStaticIdentifier())
		.data<&LoginResponse::m_Result>(Hash32Fnv1aConst("Result"))
		.data<&LoginResponse::m_SessionToken>(Hash32Fnv1aConst("SessionToken"))
		.data<&LoginResponse::m_DiscordID>(Hash32Fnv1aConst("DiscordID"))
		.data<&LoginResponse::m_DiscordAvatarHash>(Hash32Fnv1aConst("DiscordAvatarHash"))
		.data<&LoginResponse::m_Games>(Hash32Fnv1aConst("Games"))
		.data<&LoginResponse::m_Cheats>(Hash32Fnv1aConst("Cheats"));
}
