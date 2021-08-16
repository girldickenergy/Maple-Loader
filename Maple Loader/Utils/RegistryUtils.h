#pragma once

#include <string>
#include <Windows.h>
#include "../AntiDebug/xorstr.hpp"
#include "../../ThemidaSDK.h"

#pragma optimize("", off) // Optimizing might mess up Themida macros
class RegistryUtils {
public:
	/**
	 * @brief Create a registry key with the specified name
	 *
	 * @param key => Name of the registry key to create
	 */
	static void CreateKey(const std::string key)
	{
		VM_EAGLE_BLACK_START
		HKEY hKey;
		DWORD dwRet = RegCreateKeyA(HKEY_LOCAL_MACHINE, key.c_str(), &hKey);

		int error = GetLastError();
		
		// Close the handle :)
		RegCloseKey(hKey);
		VM_EAGLE_BLACK_END
	}

	/**
	 * @brief Writes a value to the specified registry key
	 *
	 * @param key => Name of the registry key to write to
	 * @param contents => The contents that are supposed to be written to the registry key
	 */
	static bool WriteValueToKey(const std::string key, const std::string contents)
	{
		VM_EAGLE_BLACK_START
		LONG nError = RegSetValueExW(HKEY_LOCAL_MACHINE, std::wstring(key.begin(), key.end()).c_str(), NULL, REG_SZ, (LPBYTE)contents.c_str(), contents.size());
		if (nError == ERROR_FILE_NOT_FOUND)
		{
			// The registry key does not exist, let's create it :)
			CreateKey(key);

			// Now that the key has been created, let's call this function again.
			WriteValueToKey(key, contents);
		}
		else if (nError == ERROR_SUCCESS)
		{
			return true;
		}
		return false;
		VM_EAGLE_BLACK_END
	}

	/**
	 * @brief Reads a value to the specified registry key
	 *
	 * @param key => Name of the registry key to read from
	 * @returns Returns a string, containing the contents from the specified key
	 */
	static std::string GetValueFromKey(const std::string key)
	{
		VM_EAGLE_BLACK_START
		STR_ENCRYPT_START
		DWORD bufferSize = 128;
		char buffer[128]{};
		DWORD type = REG_SZ;

		DWORD dwRet = RegQueryValueEx(HKEY_LOCAL_MACHINE, std::wstring(key.begin(), key.end()).c_str(), NULL, &type, (LPBYTE)&buffer, (LPDWORD)&bufferSize);

		if (dwRet == ERROR_FILE_NOT_FOUND)
			return xor("error: key not found");

		if (dwRet == ERROR_MORE_DATA)
			return xor ("error: insufficient memory");

		if (dwRet == ERROR_SUCCESS)
			return buffer;
		return xor ("error: unexpected error");
		STR_ENCRYPT_END
		VM_EAGLE_BLACK_END
	}
};

#pragma optimize("", on)