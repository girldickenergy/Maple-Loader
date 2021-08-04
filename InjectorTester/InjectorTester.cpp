// InjectorTester.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <algorithm>
#include <fstream>

#include "MemoryUtils.h"

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

void EnableDebugPriv()
{
	HANDLE hToken;
	LUID luid;
	TOKEN_PRIVILEGES tkp;

	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

	LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = luid;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges(hToken, false, &tkp, sizeof(tkp), NULL, NULL);

	CloseHandle(hToken);
}

int main()
{
	EnableDebugPriv();
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

	auto* pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(hProcess);
	auto* pNtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<LPBYTE>(hProcess) + pDosHeader->
		e_lfanew);
	
	void* maple = reinterpret_cast<void*>(MemoryUtils::FindSignature("\x61\x7A\x75\x6B\x69\x5F\x6D\x61\x67\x69\x63\xFF\xFF\xFF\xFF",
		"xxxxxxxxxxxxxxx", (uintptr_t)hProcess,
		pNtHeaders->OptionalHeader.SizeOfImage));

	void* userData = reinterpret_cast<void*>(MemoryUtils::FindSignature("\x63\x69\x67\x61\x6d\x5f\x69\x6b\x75\x7a\x61\xFF\xFF\xFF\xFF",
		"xxxxxxxxxxxxxxx", (uintptr_t)hProcess,
		pNtHeaders->OptionalHeader.SizeOfImage));

	// write maple binary
	WriteProcessMemory(hProcess, maple, buffer.data(), buffer.size(), nullptr);

	// write user data
	WriteProcessMemory(hProcess, userData, userD.c_str(), userD.size(), nullptr);
}
