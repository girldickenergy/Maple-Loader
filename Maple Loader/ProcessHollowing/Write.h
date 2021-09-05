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

	/**
	 * Scans memory to find the UserData pointer
	 *
	 * @param moduleInfo Pointer to the ModuleInfo from the Injector Stub
	 * @param pmc Pointer to the ProcessMemoryInfo from the Injector Stub
	 * @param hProcess Handle of the Injector Stub
	 * 
	 * @returns Pointer to the UserData array
	 */
	static char* GetUserDataPointer(MODULEINFO* moduleInfo, PROCESS_MEMORY_COUNTERS_EX* pmc, HANDLE hProcess)
	{
		return reinterpret_cast<char*>(MemoryUtils::ScanEx(xor ("\x63\x69\x67\x61\x6d\x5f\x69\x6b\x75\x7a\x61\xFF\xAD\xFD\xAA\xFF"), xor ("xxxxxxxxxxxxxxxx"),
			reinterpret_cast<char*>(memoryRegions[0].BaseAddress),
			reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(moduleInfo->lpBaseOfDll) + static_cast<uintptr_t>(pmc->
				PeakWorkingSetSize)), hProcess));
	}
public:
	/**
	 * Writes all data to the hollowed/RunPE'd process.
	 *
	 * @param binary Binary data which will be written to the process.
	 * @param mc MatchedClient instance which houses user-data.
	 */
	static bool WriteData(HANDLE hProcess, std::vector<unsigned char>* binary, MatchedClient* mc, int* code)
	{
		VM_FISH_RED_START
		STR_ENCRYPT_START
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

		char* userDataPointer = GetUserDataPointer(&moduleInfo, &pmc, hProcess);

		SIZE_T mapleBinaryBytesWritten = 0;
		WriteProcessMemory(hProcess, mapleBinaryPointer + 0x04, binary->data(), binary->size(), &mapleBinaryBytesWritten);

		if (mapleBinaryBytesWritten != binary->size())
		{
			TerminateProcess(hProcess, 0);
			*code = 15;
			return false;
		}

		std::vector<unsigned char> userDataBytes = std::vector<unsigned char>();
		//TODO: Make this prettier :)
		for (const auto& c : mc->username)
			userDataBytes.push_back(c);
		for (const auto& c : "0xdeadbeef")
			if (c != 0)
				userDataBytes.push_back(c);
		for (const auto& c : mc->sessionToken)
			userDataBytes.push_back(c);
		for (const auto& c : "0xdeadbeef") // Keep this as an ending, so it's easier to work with later
			userDataBytes.push_back(c);

		int userDataAttempts = 0;
		SIZE_T userDataBytesWritten = 0;
		WriteProcessMemory(hProcess, userDataPointer, userDataBytes.data(), userDataBytes.size(), &userDataBytesWritten);

		while (userDataBytesWritten != userDataBytes.size())
		{
			if (userDataAttempts < 3)
			{
				userDataPointer = GetUserDataPointer(&moduleInfo, &pmc, &hProcess);
				userDataBytesWritten = 0;
				WriteProcessMemory(hProcess, userDataPointer, userDataBytes.data(), userDataBytes.size(), &userDataBytesWritten);
				userDataAttempts++;
			}
			else
			{
				TerminateProcess(hProcess, 0);
				*code = 16;
				return false;
			}
		}

		SIZE_T synchBytesWritten = 0;
		DWORD synch_to_write = 0x13371337;
		WriteProcessMemory(hProcess, mapleBinaryPointer, &synch_to_write, 0x4, &synchBytesWritten);

		if (synchBytesWritten != 0x4)
		{
			TerminateProcess(hProcess, 0);
			*code = 17;
			return false;
		}
		STR_ENCRYPT_END
		VM_FISH_RED_END
		return true;
	}
};