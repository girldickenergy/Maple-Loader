#pragma once

#include <vector>
#include <string>

class LoginRequest
{
	std::string username;
	std::string password;
	std::string loaderVersion;
	std::string hwid;
public:
	LoginRequest(const std::string& username, const std::string& password, const std::string& loaderVersion, const std::string& hwid);
	std::vector<unsigned char> Serialize();
};