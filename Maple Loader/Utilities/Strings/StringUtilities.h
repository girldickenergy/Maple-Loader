#pragma once

#include <string>
#include <vector>

class StringUtilities
{
public:
	static std::vector<std::string> Split(std::string s, std::string delimiter);
	static std::vector<unsigned char> StringToByteArray(const std::string str);
	static std::string ByteArrayToString(const std::vector<unsigned char> arr);
};
