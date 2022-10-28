#pragma once

#include <string>

class HardwareUtilities
{
	static std::string getGPUModel();
	static std::string getCPUVendor();
	static std::string getMotherboardInfo();
public:
	static std::string GetPCName();
	static std::string GetHWID();
};
