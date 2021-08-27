#pragma once

enum class CheatStatus
{
	UpToDate = 0,
	Outdated = 1,
	Detected = 2
};

struct Cheat
{
	int CurrentStream = 0;
	
	int ID;
	int GameID;
	std::vector<std::string> ReleaseStreams;
	std::string Name;
	int Price;
	CheatStatus Status;
	std::string Features;
	std::string ExpiresAt;

	Cheat(int id, int gameID, std::vector<std::string> releaseStreams, std::string name, int price, CheatStatus status, std::string features, std::string expiresAt)
	{
		ID = id;
		GameID = gameID;
		ReleaseStreams = releaseStreams;
		Name = name;
		Price = price;
		Status = status;
		Features = features;
		ExpiresAt = expiresAt;
	}
};