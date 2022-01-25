#include "StringUtilities.h"

std::vector<std::string> StringUtilities::Split(const std::string& s, const std::string& delimiter)
{
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));

    return res;
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
