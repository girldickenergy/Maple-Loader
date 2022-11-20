#pragma once

#include <vector>
#include <string>

#include "CheatStatus.h"

class Cheat
{
	unsigned int id;
	unsigned int gameID;
	std::string name;
	std::vector<std::string> releaseStreams;
	unsigned int startingPrice;
	CheatStatus status;
	std::string expiresOn;
public:
	int CurrentStream = 0;

	Cheat(unsigned int id, unsigned int gameID, const std::string& name, const std::vector<std::string> releaseStreams, unsigned int startingPrice, CheatStatus status, const std::string& expiresOn);
	Cheat() = default;

	unsigned int GetID();
	unsigned int GetGameID();
	const std::string& GetName();
	std::vector<std::string>& GetReleaseStreams();
	unsigned int GetStartingPrice();
	CheatStatus GetStatus();
	const std::string& GetExpiration();
};