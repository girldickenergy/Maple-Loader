#pragma once

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <algorithm>
#include <fstream>
#include <tchar.h>
#include <psapi.h>
#include <stdexcept>
#include "../Communication/MatchedClient.h"
#include "../AntiDebug/xorstr.hpp"

namespace RunPE
{
	struct MemoryRegion
	{
		DWORD BaseAddress;
		SIZE_T RegionSize;
		DWORD State;
		DWORD Protect;
		DWORD Type;

		explicit MemoryRegion(MEMORY_BASIC_INFORMATION32 mbi)
		{
			BaseAddress = mbi.BaseAddress;
			RegionSize = mbi.RegionSize;
			State = mbi.State;
			Protect = mbi.Protect;
			Type = mbi.Type;
		}
	};
	
	class Write
	{
	private:
		/**
		 * \brief Cached memory regions from the hollowed/RunPE'd process.
		 */
		static inline std::vector<MemoryRegion> memoryRegions;
		/**
		 * Finds process ID by name
		 *
		 * @param pName The Process name with extension
		 * @return Process ID
		 */
		static auto FindProcessId(const std::string& pName) -> DWORD
		{
			PROCESSENTRY32 processInfo;
			processInfo.dwSize = sizeof processInfo;
			const std::wstring processName(pName.begin(), pName.end());

			const HANDLE processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
			if (processSnapshot == INVALID_HANDLE_VALUE)
			{
				return 0;
			}

			Process32First(processSnapshot, &processInfo);
			if (_wcsnicmp(processInfo.szExeFile, processName.c_str(), wcslen(processName.c_str())) == 0)
			{
				CloseHandle(processSnapshot);
				return processInfo.th32ProcessID;
			}

			while (Process32Next(processSnapshot, &processInfo))
			{
				if (_wcsnicmp(processInfo.szExeFile, processName.c_str(), wcslen(processName.c_str())) == 0)
				{
					CloseHandle(processSnapshot);
					return processInfo.th32ProcessID;
				}
			}

			CloseHandle(processSnapshot);
			return 0;
		}

		/**
		 * Caches all the memory regions
		 *
		 * @param hProcess Handle to the process
		 */
		static void CacheMemoryRegions(HANDLE hProcess)
		{
			memoryRegions.clear();

			MEMORY_BASIC_INFORMATION32 mbi;
			LPCVOID address = nullptr;

			while (VirtualQueryEx(hProcess, address, reinterpret_cast<PMEMORY_BASIC_INFORMATION>(&mbi),
				sizeof mbi) != 0)
			{
				if (mbi.State == MEM_COMMIT && mbi.Protect >= 0x10 && mbi.Protect <= 0x80)
				{
					memoryRegions.push_back(*new MemoryRegion(mbi));
				}
				address = reinterpret_cast<LPCVOID>(mbi.BaseAddress + mbi.RegionSize);
			}
		}

		/**
		 * Scan the process space for an Array of bytes
		 *
		 * @param pattern The signature/Array of bytes in string form
		 * @param mask The mask of the signature. ? -> wildcard | x -> static byte
		 * @param begin The address from where to begin searching
		 * @param size Total size to search in
		 * @return Address of Array of bytes
		 */
		static auto ScanIn(const char* pattern, const char* mask, char* begin, unsigned int size) -> char*
		{
			const unsigned int patternLength = strlen(mask);

			for (unsigned int i = 0; i < size - patternLength; i++)
			{
				bool found = true;
				for (unsigned int j = 0; j < patternLength; j++)
				{
					if (mask[j] != '?' && pattern[j] != *(begin + i + j))
					{
						found = false;
						break;
					}
				}
				if (found)
				{
					return (begin + i);
				}
			}
			return nullptr;
		}

		/**
		 * External wrapper for the 'ScanIn' function
		 *
		 * @param pattern The signature/Array of bytes in string form
		 * @param mask The mask of the signature. ? -> wildcard | x -> static byte
		 * @param begin The address from where to begin searching
		 * @param end The address where searching will end
		 * @param hProc Handle to the process
		 * @return Address of Array of bytes
		 */
		static auto ScanEx(const char* pattern, const char* mask, char* begin, char* end, HANDLE hProc) -> char*
		{
			char* currentChunk = begin;
			char* match = nullptr;
			SIZE_T bytesRead;

			while (currentChunk < end)
			{
				MEMORY_BASIC_INFORMATION mbi;

				if (!VirtualQueryEx(hProc, currentChunk, &mbi, sizeof(mbi)))
				{
					int err = GetLastError();
					return nullptr;
				}

				char* buffer = new char[mbi.RegionSize];

				if (mbi.State == MEM_COMMIT && mbi.Protect != PAGE_NOACCESS)
				{
					DWORD previousProtect;
					if (VirtualProtectEx(hProc, mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &previousProtect))
					{
						ReadProcessMemory(hProc, mbi.BaseAddress, buffer, mbi.RegionSize, &bytesRead);
						VirtualProtectEx(hProc, mbi.BaseAddress, mbi.RegionSize, previousProtect, &previousProtect);

						if (char * internalAddress = ScanIn(pattern, mask, buffer, bytesRead); internalAddress != nullptr)
						{
							const uintptr_t offsetFromBuffer = internalAddress - buffer;
							match = currentChunk + offsetFromBuffer;
							delete[] buffer;
							break;
						}
					}
				}

				currentChunk = currentChunk + mbi.RegionSize;
				delete[] buffer;
			}
			return match;
		}
	public:
		/**
		 * Writes all data to the hollowed/RunPE'd process.
		 *
		 * @param binary Binary data which will be written to the process.
		 * @param mc MatchedClient instance which houses user-data.
		 */
		static void WriteData(std::vector<unsigned char> *binary, MatchedClient *mc)
		{
			//TODO: Maybe in the future this will be a randomized process?					
			const DWORD processId = FindProcessId(xor("notepad.exe"));
			const HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);

			HMODULE modules[256];
			DWORD moduleCount;
			EnumProcessModules(hProcess, modules, sizeof modules, &moduleCount);

			MODULEINFO moduleInfo;
			GetModuleInformation(hProcess, modules[0] /*First index is usually the main binary*/,
				&moduleInfo, sizeof MODULEINFO);

			PROCESS_MEMORY_COUNTERS_EX pmc;
			GetProcessMemoryInfo(hProcess, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof pmc);

			CacheMemoryRegions(hProcess);
			char* mapleBinaryPointer = ScanEx(xor("\x61\x7A\x75\x6B\x69\x5F\x6D\x61\x67\x69\x63\xFF\xAD\xFD\xAA\xFF"), xor("xxxxxxxxxxxxxxxx"),
				reinterpret_cast<char*>(memoryRegions[0].BaseAddress), 
				reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll) + static_cast<uintptr_t>(pmc.
					PeakWorkingSetSize)), hProcess);
			
			char* userDataPointer = ScanEx(xor("\x63\x69\x67\x61\x6d\x5f\x69\x6b\x75\x7a\x61\xFF\xAD\xFD\xAA\xFF"), xor("xxxxxxxxxxxxxxxx"),
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
}
