#pragma once

#include <string>

class HardwareHelper
{
	static std::string getGPUID();
	static std::string getCPUVendor();
	static std::string getMotherboardInfo();
public:
	static std::string GetHWID();
};