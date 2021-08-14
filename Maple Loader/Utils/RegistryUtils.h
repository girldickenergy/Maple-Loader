#pragma once

#include <string>
#include <Windows.h>
#include "../AntiDebug/xorstr.hpp"

class RegistryUtils {
public:
	/**
	 * Create a registry key with the specified name
	 *
	 * @param key => Name of the registry key to create
	 */
	static void CreateKey(const std::string key)
	{
		PHKEY handle{};
		DWORD dwRet = RegCreateKeyA(HKEY_LOCAL_MACHINE, key.c_str(), handle);
		
		// Close the handle :)
		RegCloseKey(reinterpret_cast<HKEY>(handle));
	}

	/**
	 * Writes a value to the specified registry key
	 *
	 * @param key => Name of the registry key to write to
	 * @param contents => The contents that are supposed to be written to the registry key
	 */
	static bool WriteValueToKey(const std::string key, const std::string contents)
	{
		DWORD BufferSize = 512;
		DWORD cbData = BufferSize;
		DWORD dwRet;
		PPERF_DATA_BLOCK PerfData = (PPERF_DATA_BLOCK)malloc(BufferSize);

		dwRet = RegQueryValueEx(HKEY_LOCAL_MACHINE, std::wstring(key.begin(), key.end()).c_str() , NULL, NULL, (LPBYTE)PerfData, &cbData);

		if (dwRet == ERROR_FILE_NOT_FOUND)
		{
			// The registry key does not exist, let's create it :)
			CreateKey(key);

			// Now that the key has been created, let's call this function again.
			WriteValueToKey(key, contents);
		}
		else if (dwRet == ERROR_SUCCESS)
		{
			return true;
		}
		return false;
	}

	/**
	 * Reads a value to the specified registry key
	 *
	 * @param key => Name of the registry key to read from
	 * @returns Returns a string, containing the contents from the specified key
	 */
	static std::string GetValueFromKey(const std::string key)
	{
		DWORD BufferSize = 512;
		DWORD cbData = BufferSize;
		DWORD dwRet;
		PPERF_DATA_BLOCK PerfData = (PPERF_DATA_BLOCK)malloc(BufferSize);

		dwRet = RegQueryValueEx(HKEY_LOCAL_MACHINE, std::wstring(key.begin(), key.end()).c_str(), NULL, NULL, (LPBYTE)PerfData, &cbData);

		if (dwRet == ERROR_FILE_NOT_FOUND)
			return xor("error: key not found");

		while (dwRet == ERROR_MORE_DATA)
		{
			BufferSize += 24;
			PerfData = (PPERF_DATA_BLOCK)realloc(PerfData, BufferSize);

			dwRet = RegQueryValueEx(HKEY_LOCAL_MACHINE, std::wstring(key.begin(), key.end()).c_str(), NULL, NULL, (LPBYTE)PerfData, &cbData);
		}

		if (dwRet == ERROR_SUCCESS)
			return key;
		return xor ("error: unexpected error");
	}
};