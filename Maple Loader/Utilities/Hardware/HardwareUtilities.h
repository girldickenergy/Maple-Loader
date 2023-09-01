#pragma once

#include <string>

class HardwareUtilities
{
	static std::string getGPUModel();
	static std::string getCPUVendor();
	static std::string getMotherboardInfo();
	static std::string getUserName();
	static std::string getWindowsInstallDate();
	static std::string getWindowsProductName();
public:
	static std::string GetPCName();
	static std::string GetHWID();
};
