#include "AutofillUtilities.h"

#include <windows.h>

#include "../Security/xorstr.hpp"
#include "../Crypto/CryptoUtilities.h"
#include "../Hardware/HardwareUtilities.h"
#include "../Strings/StringUtilities.h"

std::pair<std::string, std::string> AutofillUtilities::GetCredentials()
{
	const std::string uniqueName = CryptoUtilities::GetMD5Hash(HardwareUtilities::GetPCName() + xorstr_("_MapleLoader"));
	const std::string registryPath = xorstr_("SOFTWARE\\") + CryptoUtilities::GetMD5Hash(HardwareUtilities::GetPCName() + xorstr_("_MapleLoader"));

	HKEY hKey;
	if (RegOpenKeyA(HKEY_CURRENT_USER, registryPath.c_str(), &hKey) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);

		return std::make_pair<std::string, std::string>("", "");
	}

	DWORD bufferSize = 293;
	char buffer[293]{};

	if (RegQueryValueA(hKey, xorstr_("dat"), buffer, (PLONG)&bufferSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);

		return std::make_pair<std::string, std::string>("", "");
	}

	RegCloseKey(hKey);

	const int xorKey = uniqueName[uniqueName.length() / 2];
	std::string data;
	for (const auto& c : std::string(buffer))
		data.push_back((c ^ xorKey) == 0 ? c : c ^ xorKey);

	const std::vector<std::string> splitData = StringUtilities::Split(data, xorstr_("(industrybaby)"));
	if (splitData.size() != 2)
		return std::make_pair<std::string, std::string>("", "");

	return std::make_pair<std::string, std::string>(splitData[0].c_str(), splitData[1].c_str());
}

void AutofillUtilities::RememberCredentials(const std::string& username, const std::string& password)
{
	const std::string uniqueName = CryptoUtilities::GetMD5Hash(HardwareUtilities::GetPCName() + xorstr_("_MapleLoader"));
	const std::string registryPath = xorstr_("SOFTWARE\\") + CryptoUtilities::GetMD5Hash(HardwareUtilities::GetPCName() + xorstr_("_MapleLoader"));

	HKEY hKey;
	if (RegOpenKeyA(HKEY_CURRENT_USER, registryPath.c_str(), &hKey) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);

		if (!SUCCEEDED(RegCreateKeyA(HKEY_CURRENT_USER, registryPath.c_str(), &hKey)))
		{
			RegCloseKey(hKey);

			return;
		}
	}

	int xorKey = uniqueName[uniqueName.length() / 2];
	std::string data;
	for (const auto& c : username + std::string(xorstr_("(industrybaby)")) + password)
		if (c != 0)
			data.push_back((c ^ xorKey) == 0 ? c : c ^ xorKey);
	data.push_back('\0');

	if (RegSetValueA(hKey, xorstr_("dat"), REG_SZ, data.c_str(), data.length()) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);

		return;
	}

	RegCloseKey(hKey);
}