#include "Cheat.h"

Cheat::Cheat(unsigned int id, unsigned int gameID, const std::string& name, const std::vector<std::string> releaseStreams, unsigned int startingPrice, CheatStatus status, const std::string& expiresOn)
{
	this->id = id;
	this->gameID = gameID;
	this->name = name;
	this->releaseStreams = releaseStreams;
	this->startingPrice = startingPrice;
	this->status = status;
	this->expiresOn = expiresOn;
}

unsigned int Cheat::GetID()
{
	return id;
}

unsigned int Cheat::GetGameID()
{
	return gameID;
}

const std::string& Cheat::GetName()
{
	return name;
}

std::vector<std::string>& Cheat::GetReleaseStreams()
{
	return releaseStreams;
}

unsigned int Cheat::GetStartingPrice()
{
	return startingPrice;
}

CheatStatus Cheat::GetStatus()
{
	return status;
}

const std::string& Cheat::GetExpiration()
{
	return expiresOn;
}
