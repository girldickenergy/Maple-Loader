#pragma once

#include "../TCP/TCP_Client.h"
#include "../Crypto/Wrapper/AES.h"

class MatchedClient
{
public:
	TcpClient client;
	AESWrapper* aes;
	std::string sessionToken;

	MatchedClient(TcpClient _client)
	{
		client = _client;
		aes = new AESWrapper();
		sessionToken = "";
	}
};