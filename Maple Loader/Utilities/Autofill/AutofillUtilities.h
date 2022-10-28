#pragma once
#include <string>

class AutofillUtilities
{
public:
	static std::pair<std::string, std::string> GetCredentials();
	static void RememberCredentials(const std::string& username, const std::string& password);
};
