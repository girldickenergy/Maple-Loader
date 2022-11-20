#include "StringUtilities.h"

std::vector<std::string> StringUtilities::Split(std::string s, std::string delimiter)
{
	size_t pos = 0;
	std::string token;
	std::vector<std::string> ret;
	while ((pos = s.find(delimiter)) != std::string::npos) {
		token = s.substr(0, pos);
		ret.push_back(token);
		s.erase(0, pos + delimiter.length());
	}
	ret.push_back(s);

	return ret;
}

std::vector<unsigned char> StringUtilities::StringToByteArray(const std::string str)
{
    std::vector<unsigned char> arr;
    for (const auto& c : str)
        arr.push_back(c);

    return arr;
}

std::string StringUtilities::ByteArrayToString(const std::vector<unsigned char> arr)
{
    std::string str;
    for (const auto& c : arr)
        str.push_back(c);

    return str;
}