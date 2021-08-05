// InjectorTester.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <algorithm>
#include <fstream>
#include <tchar.h>
#include <psapi.h>

#include "MemoryUtils.h"

HMODULE GetBaseAddressByName(DWORD processId, const TCHAR* processName)
{
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, processId);

	if (NULL != hProcess)
	{
		HMODULE hMod;
		DWORD cbNeeded;

		if (EnumProcessModulesEx(hProcess, &hMod, sizeof(hMod),
			&cbNeeded, LIST_MODULES_32BIT | LIST_MODULES_64BIT))
		{
			GetModuleBaseName(hProcess, hMod, szProcessName,
				sizeof(szProcessName) / sizeof(TCHAR));
			if (!_tcsicmp(processName, szProcessName)) {
				return hMod;
				_tprintf(TEXT("0x%p\n"), hMod);
			}
		}
	}

	CloseHandle(hProcess);
	return (HMODULE)0xDEADBEEF;
}

auto FindProcessId(const std::wstring& processName) -> DWORD
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof processInfo;

	HANDLE processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processSnapshot == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	Process32First(processSnapshot, &processInfo);
	if (processName.compare(processInfo.szExeFile) == 0)
	{
		CloseHandle(processSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(processSnapshot, &processInfo))
	{
		if (processName.compare(processInfo.szExeFile) == 0)
		{
			CloseHandle(processSnapshot);
			return processInfo.th32ProcessID;
		}
	}

	CloseHandle(processSnapshot);
	return 0;
}

char* ScanIn(const char* pattern, const char* mask, char* begin, unsigned int size)
{
	unsigned int patternLength = strlen(mask);

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

char* ScanEx(const char* pattern, const char* mask, char* begin, char* end, HANDLE hProc)
{
	char* currentChunk = begin;
	char* match = nullptr;
	SIZE_T bytesRead;

	while (currentChunk < end)
	{
		MEMORY_BASIC_INFORMATION mbi;

		//return nullptr if VirtualQuery fails
		if (!VirtualQueryEx(hProc, currentChunk, &mbi, sizeof(mbi)))
		{
			int err = GetLastError();
			return nullptr;
		}

		char* buffer = new char[mbi.RegionSize];

		if (mbi.State == MEM_COMMIT && mbi.Protect != PAGE_NOACCESS)
		{
			DWORD oldprotect;
			if (VirtualProtectEx(hProc, mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &oldprotect))
			{
				ReadProcessMemory(hProc, mbi.BaseAddress, buffer, mbi.RegionSize, &bytesRead);
				VirtualProtectEx(hProc, mbi.BaseAddress, mbi.RegionSize, oldprotect, &oldprotect);

				char* internalAddress = ScanIn(pattern, mask, buffer, bytesRead);

				if (internalAddress != nullptr)
				{
					//calculate from internal to external
					uintptr_t offsetFromBuffer = internalAddress - buffer;
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

inline std::vector<MemoryRegion> memoryRegions;

void cacheMemoryRegions(HANDLE hProcess)
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


int main()
{
	std::string file = "C:\\Cheats\\0.dll";

	std::ifstream inFile(file, std::ios_base::binary);

	inFile.seekg(0, std::ios_base::end);
	size_t length = inFile.tellg();
	inFile.seekg(0, std::ios_base::beg);

	std::vector<unsigned char> buffer;
	buffer.reserve(length);
	std::copy(std::istreambuf_iterator<char>(inFile),
		std::istreambuf_iterator<char>(),
		std::back_inserter(buffer));

	std::string userD = "something something i dont really care lol";

	DWORD ProcessId = FindProcessId(L"Injector.exe");
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);

	HMODULE modules[250];
	DWORD cbNeeded;
	EnumProcessModules(hProcess, modules, sizeof(modules), &cbNeeded);

	MODULEINFO mi;
	GetModuleInformation(hProcess, modules[0], &mi, sizeof MODULEINFO);

	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof pmc);

	cacheMemoryRegions(hProcess);
	
	char* maple = ScanEx("\x61\x7A\x75\x6B\x69\x5F\x6D\x61\x67\x69\x63\xFF\xAD\xFD\xAA\xFF", "xxxxxxxxxxxxxxxx", 
		(char*)memoryRegions[0].BaseAddress, (char*)((uintptr_t)mi.lpBaseOfDll + (uintptr_t)pmc.PeakWorkingSetSize), hProcess);
	char* userData = ScanEx("\x63\x69\x67\x61\x6d\x5f\x69\x6b\x75\x7a\x61\xFF\xAD\xFD\xAA\xFF", "xxxxxxxxxxxxxxxx", 
		(char*)memoryRegions[0].BaseAddress, (char*)((uintptr_t)mi.lpBaseOfDll + (uintptr_t)pmc.PeakWorkingSetSize), hProcess);

	// write maple binary
 	WriteProcessMemory(hProcess, maple, buffer.data(), buffer.size(), nullptr);

	// write user data
 	WriteProcessMemory(hProcess, userData, userD.c_str(), userD.size(), nullptr);
}
