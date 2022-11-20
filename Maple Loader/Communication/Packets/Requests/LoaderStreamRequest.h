#pragma once

#include <string>
#include <vector>

class LoaderStreamRequest
{
	std::string sessionToken;
	unsigned int cheatID;
public:
	LoaderStreamRequest(const std::string& sessionToken, unsigned int cheatID);
	std::vector<unsigned char> Serialize();
};