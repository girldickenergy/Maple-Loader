#include "Cheat.h"

#include "entt.hpp"
#include "Fnv1a.h"

#include "../Utilities/Reflection/TypeRegistrar.h"

static const TypeRegistrar<Cheat> registrar;

uint32_t Cheat::GetID()
{
	return m_ID;
}

uint32_t Cheat::GetGameID()
{
	return m_GameID;
}

const std::string& Cheat::GetName()
{
	return m_Name;
}

std::vector<std::string>& Cheat::GetReleaseStreams()
{
	return m_ReleaseStreams;
}

uint32_t Cheat::GetStartingPrice()
{
	return m_StartingPrice;
}

CheatStatus Cheat::GetStatus()
{
	return static_cast<CheatStatus>(m_Status);
}

const std::string& Cheat::GetExpiration()
{
	return m_ExpiresOn;
}

void Cheat::Register()
{
	if (registrar.IsRegistered)
		return;

	entt::meta<Cheat>()
		.data<&Cheat::m_ID>(Hash32Fnv1aConst("ID"))
		.data<&Cheat::m_GameID>(Hash32Fnv1aConst("GameID"))
		.data<&Cheat::m_Name>(Hash32Fnv1aConst("Name"))
		.data<&Cheat::m_ReleaseStreams>(Hash32Fnv1aConst("ReleaseStreams"))
		.data<&Cheat::m_StartingPrice>(Hash32Fnv1aConst("StartingPrice"))
		.data<&Cheat::m_Status>(Hash32Fnv1aConst("Status"))
		.data<&Cheat::m_ExpiresOn>(Hash32Fnv1aConst("ExpiresOn"));
}
