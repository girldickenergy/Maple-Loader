#pragma once

#include <string>

class CryptoHelper
{
public:
	static std::string GetMD5Hash(const std::string& str);
	static std::string GetSHA256HashOfCurrentFile();
};