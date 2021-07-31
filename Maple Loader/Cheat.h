#pragma once

enum class CheatStatus
{
	UpToDate = 0,
	Outdated = 1,
	Detected = 2
};

struct Cheat
{
	int ID;
	int GameID;
	std::string Name;
	int Price;
	CheatStatus Status;
	std::string Features;
	std::string ExpiresAt;

	Cheat(int id, int gameID, std::string name, int price, CheatStatus status, std::string features, std::string expiresAt)
	{
		ID = id;
		GameID = gameID;
		Name = name;
		Price = price;
		Status = status;
		Features = features;
		ExpiresAt = expiresAt;
	}
};