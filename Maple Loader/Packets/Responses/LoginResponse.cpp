#include "LoginResponse.h"

#include "../../Utils/StringUtilities.h"

LoginResponse::LoginResponse(const char* msg, size_t size, MatchedClient* matchedClient) : Response(msg, size)
{
	//login result
	auto encryptedLoginResult = StringUtilities::StringToByteArray(RawData[0]);
	
	encryptedLoginResult.erase(encryptedLoginResult.begin());

	std::string decryptedLoginResult = matchedClient->aes->Decrypt(encryptedLoginResult);

	std::vector<std::string> decryptedLoginResultSplit = StringUtilities::Split(decryptedLoginResult);
	Result = static_cast<LoginResult>(decryptedLoginResultSplit[0][0]);
	if (Result != LoginResult::Success)  {
		matchedClient->username = "";
		return;
	}

	decryptedLoginResultSplit[1].erase(decryptedLoginResultSplit[1].begin());
	SessionToken = decryptedLoginResultSplit[1];
	matchedClient->sessionToken = SessionToken;
	//login result

	//games list
	auto encryptedGames = StringUtilities::StringToByteArray(RawData[1]);

	encryptedGames.erase(encryptedGames.begin());

	std::string decryptedGames = matchedClient->aes->Decrypt(encryptedGames);
	std::vector<std::string> decryptedGamesSplit = StringUtilities::Split(decryptedGames);
	for (int i = 0; i < decryptedGamesSplit.size(); i++)
	{
		if (i > 0)
			decryptedGamesSplit[i].erase(decryptedGamesSplit[i].begin());
		
		std::vector<std::string> gameSplit = StringUtilities::Split(matchedClient->aes->Decrypt(StringUtilities::StringToByteArray(decryptedGamesSplit[i])));
		gameSplit[1].erase(gameSplit[1].begin());
		gameSplit[2].erase(gameSplit[2].begin());

		Games.push_back(new Game(std::stoi(gameSplit[0]), gameSplit[1], gameSplit[2]));
	}
	//games list
	
	//cheats list
	auto encryptedCheats = StringUtilities::StringToByteArray(RawData[2]);

	encryptedCheats.erase(encryptedCheats.begin());

	std::string decryptedCheats = matchedClient->aes->Decrypt(encryptedCheats);
	std::vector<std::string> decryptedCheatsSplit = StringUtilities::Split(decryptedCheats);
	for (int i = 0; i < decryptedCheatsSplit.size(); i++)
	{
		if (i > 0)
			decryptedCheatsSplit[i].erase(decryptedCheatsSplit[i].begin());

		std::vector<std::string> cheatSplit = StringUtilities::Split(matchedClient->aes->Decrypt(StringUtilities::StringToByteArray(decryptedCheatsSplit[i])));
		for (int j = 1; j < cheatSplit.size(); j++)
			cheatSplit[j].erase(cheatSplit[j].begin());

		Cheats.push_back(new Cheat(std::stoi(cheatSplit[0]), std::stoi(cheatSplit[1]), StringUtilities::Split(cheatSplit[2], ','), cheatSplit[3], std::stoi(cheatSplit[4]), static_cast<CheatStatus>(std::stoi(cheatSplit[5])), cheatSplit[6], cheatSplit[7]));
	}
	//cheats list
}
