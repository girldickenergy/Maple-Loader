#include "AutofillHelper.h"

#include <filesystem>
#include <fstream>

#include "../Crypto/CryptoHelper.h"
#include "../GeneralHelper.h"
#include "../Security/xorstr.hpp"
#include "../Strings/StringUtilities.h"
#include "../../Communication/Communication.h"

void AutofillHelper::Fill()
{
	std::string uniqueName = CryptoHelper::GetMD5Hash(GeneralHelper::GetPCName() + xor ("_MapleLoader"));
	std::string registryPath = xor ("SOFTWARE\\") + CryptoHelper::GetMD5Hash(GeneralHelper::GetPCName() + xor ("_MapleLoader"));

	HKEY hKey;
	if (RegOpenKeyA(HKEY_CURRENT_USER, registryPath.c_str(), &hKey) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);

		return;
	}

	DWORD bufferSize = 293;
	char buffer[293]{};

	if (RegQueryValueA(hKey, xor ("dat"), buffer, (PLONG)&bufferSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);

		return;
	}

	RegCloseKey(hKey);

	int xorKey = uniqueName[uniqueName.length() / 2];
	std::string data;
	for (const auto& c : std::string(buffer))
		data.push_back((c ^ xorKey) == 0 ? c : c ^ xorKey);

	std::vector<std::string> splitData = StringUtilities::Split(data, xor ("(industrybaby)"));
	if (splitData.size() == 2)
	{
		memcpy(Communication::CurrentUser->Username, splitData[0].c_str(), splitData[0].length());
		memcpy(Communication::CurrentUser->Password, splitData[1].c_str(), splitData[1].length());
	}
}

void AutofillHelper::Remember()
{
	std::string uniqueName = CryptoHelper::GetMD5Hash(GeneralHelper::GetPCName() + xor ("_MapleLoader"));
	std::string registryPath = xor ("SOFTWARE\\") + CryptoHelper::GetMD5Hash(GeneralHelper::GetPCName() + xor ("_MapleLoader"));

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
	for (const auto& c : Communication::CurrentUser->Username + std::string(xor ("(industrybaby)")) + Communication::CurrentUser->Password)
		if (c != 0)
			data.push_back((c ^ xorKey) == 0 ? c : c ^ xorKey);
	data.push_back('\0');

	if (RegSetValueA(hKey, xor ("dat"), REG_SZ, data.c_str(), data.length()) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);

		return;
	}

	RegCloseKey(hKey);
}
