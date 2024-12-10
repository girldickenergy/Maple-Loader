#pragma once

#include <vector>
#include <string>

#include "CheatStatus.h"

class Cheat
{
	uint32_t m_ID;
	uint32_t m_GameID;
	std::string m_Name;
	std::vector<std::string> m_ReleaseStreams;
	uint32_t m_StartingPrice;
	uint32_t m_Status;
	std::string m_ExpiresOn;
public:
	int CurrentStream = 0;

	Cheat() = default;

	unsigned int GetID();
	unsigned int GetGameID();
	const std::string& GetName();
	std::vector<std::string>& GetReleaseStreams();
	unsigned int GetStartingPrice();
	CheatStatus GetStatus();
	const std::string& GetExpiration();

	static void Register();
};