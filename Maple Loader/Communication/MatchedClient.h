#pragma once

#include "../TCP/TCP_Client.h"
#include "../Crypto/Wrapper/AES.h"

class MatchedClient
{
public:
	TcpClient client;
	AESWrapper* aes;
	std::string sessionToken;
	std::string username;
	std::string discordID;
	std::string avatarHash;

	MatchedClient(TcpClient _client)
	{
		client = _client;
		aes = new AESWrapper();
		sessionToken = "";
		username = "";
		discordID = "";
		avatarHash = "";
	}
};