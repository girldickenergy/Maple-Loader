#pragma once

#include <string>

class GeneralHelper
{
public:
	static void ShutdownAndExit(bool disconnect = true);
	static std::string GetPCName();
};