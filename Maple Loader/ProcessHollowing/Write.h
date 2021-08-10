#pragma once

#include <psapi.h>

#include "MemoryUtils.h"
#include "../AntiDebug/xorstr.hpp"

class Write
{
	/**
	 * \brief Cached memory regions from the hollowed/RunPE'd process.
	 */
	static inline std::vector<MemoryRegion> memoryRegions;
public:
	/**
	 * Writes all data to the hollowed/RunPE'd process.
	 *
	 * @param binary Binary data which will be written to the process.
	 * @param mc MatchedClient instance which houses user-data.
	 */
	static void WriteData(HANDLE hProcess, std::vector<unsigned char>* binary, MatchedClient* mc)
	{
		HMODULE modules[256];
		DWORD moduleCount;
		EnumProcessModules(hProcess, modules, sizeof modules, &moduleCount);

		MODULEINFO moduleInfo;
		GetModuleInformation(hProcess, modules[0] /*First index is usually the main binary*/,
			&moduleInfo, sizeof MODULEINFO);

		PROCESS_MEMORY_COUNTERS_EX pmc;
		GetProcessMemoryInfo(hProcess, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof pmc);

		memoryRegions = MemoryUtils::GetMemoryRegions(hProcess);
		char* mapleBinaryPointer = MemoryUtils::ScanEx(xor ("\x61\x7A\x75\x6B\x69\x5F\x6D\x61\x67\x69\x63\xFF\xAD\xFD\xAA\xFF"), xor ("xxxxxxxxxxxxxxxx"),
			reinterpret_cast<char*>(memoryRegions[0].BaseAddress),
			reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll) + static_cast<uintptr_t>(pmc.
				PeakWorkingSetSize)), hProcess);

		char* userDataPointer = MemoryUtils::ScanEx(xor ("\x63\x69\x67\x61\x6d\x5f\x69\x6b\x75\x7a\x61\xFF\xAD\xFD\xAA\xFF"), xor ("xxxxxxxxxxxxxxxx"),
			reinterpret_cast<char*>(memoryRegions[0].BaseAddress),
			reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll) + static_cast<uintptr_t>(pmc.
				PeakWorkingSetSize)), hProcess);

		SIZE_T mapleBinaryBytesWritten = 0;
		WriteProcessMemory(hProcess, mapleBinaryPointer, binary->data(), binary->size(), &mapleBinaryBytesWritten);

		if (mapleBinaryBytesWritten != binary->size())
			throw std::runtime_error(xor ("Data was unable to be fully written!"));

		std::vector<unsigned char> userDataBytes = std::vector<unsigned char>();
		//TODO: Make this prettier :)
		for (const auto& c : mc->username)
			userDataBytes.push_back(c);
		for (const auto& c : "0xdeadbeef")
			userDataBytes.push_back(c);
		for (const auto& c : mc->sessionToken)
			userDataBytes.push_back(c);
		for (const auto& c : "0xdeadbeef") // Keep this as an ending, so it's easier to work with later
			userDataBytes.push_back(c);

		SIZE_T userDataBytesWritten = 0;
		WriteProcessMemory(hProcess, userDataPointer, userDataBytes.data(), userDataBytes.size(), &userDataBytesWritten);

		if (userDataBytesWritten != userDataBytes.size())
			throw std::runtime_error(xor ("Data was unable to be fully written!"));
	}
};