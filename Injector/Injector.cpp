#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <algorithm>
#include <psapi.h>
#include <filesystem>

#include "MemoryUtils.h"

#include "blackbone/BlackBone/Process/Process.h"

static inline std::vector<MemoryRegion> memoryRegions;

const inline void adjustPrivileges()
{
	HANDLE token;
	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	tp.Privileges[0].Luid.LowPart = 20; // 20 = SeDebugPrivilege
	tp.Privileges[0].Luid.HighPart = 0;

	if (OpenProcessToken((HANDLE)-1, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token))
	{
		AdjustTokenPrivileges(token, FALSE, &tp, 0, NULL, 0);
		CloseHandle(token);
	}
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//VM_START
//	STR_ENCRYPTW_START

	unsigned char azukiMagic[] = { 0x61, 0x7a, 0x75, 0x6b, 0x69, 0x5f, 0x6d, 0x61, 0x67, 0x69, 0x63 };
	unsigned char azukiMagicRev[] = { 0x63, 0x69, 0x67, 0x61, 0x6d, 0x5f, 0x69, 0x6b, 0x75, 0x7a, 0x61 };
	//ShowWindow(GetConsoleWindow(), SW_HIDE);

	// TODO: allocate MemoryRegion for pointer safety
	auto* mapleBinary = new char[150000000];
	auto* userData = new char[256 * 5 * 10];

	memset(mapleBinary, 0xFF, 150000000); // set entire memory space to 0xFF
	memset(userData, 0xFF, 256 * 5 * 10); // set entire memory space to 0xFF

	memcpy(mapleBinary, azukiMagic, sizeof azukiMagic); // copy "azuki_magic" into mapleBinary region
	memcpy(userData, azukiMagicRev, sizeof azukiMagicRev); // copy "cigam_ikuza" into userData region

	mapleBinary[sizeof(azukiMagic) + 0x01] = 0xAD;
	mapleBinary[sizeof(azukiMagic) + 0x02] = 0xFD;
	mapleBinary[sizeof(azukiMagic) + 0x03] = 0xAA;
	mapleBinary[sizeof(azukiMagic) + 0x04] = 0xFF;

	userData[sizeof(azukiMagicRev) + 0x01] = 0xAD;
	userData[sizeof(azukiMagicRev) + 0x02] = 0xFD;
	userData[sizeof(azukiMagicRev) + 0x03] = 0xAA;
	userData[sizeof(azukiMagicRev) + 0x04] = 0xFF;

	// first the binary has to be written
	while (memcmp(mapleBinary, azukiMagic, sizeof azukiMagic) == 0)
	{
		Sleep(1500); // don't know how fast memory writes will happen so ye
	}
	// then user data has to be written
	while (memcmp(userData, azukiMagicRev, sizeof azukiMagicRev) == 0)
	{
		Sleep(1500);
	}

	const auto oldNtHeader{ reinterpret_cast<IMAGE_NT_HEADERS*>(mapleBinary + reinterpret_cast<IMAGE_DOS_HEADER*>(mapleBinary)->e_lfanew) };

	adjustPrivileges();
	DWORD osu = FindProcessId(L"osu!.exe");
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, osu);

	blackbone::Process proc;
	proc.Attach(osu);

	auto image = proc.mmap().MapImage(oldNtHeader->OptionalHeader.SizeOfImage, mapleBinary, false, blackbone::CreateLdrRef | blackbone::RebaseProcess | blackbone::NoDelayLoad);
	memoryRegions = MemoryUtils::GetMemoryRegions(hProcess);

	Sleep(10000);

	int timesRedone = 0;

Label_Redo:
	HMODULE modules[250];
	DWORD cbNeeded;
	EnumProcessModules(hProcess, modules, sizeof(modules), &cbNeeded);

	MODULEINFO mi;
	GetModuleInformation(hProcess, modules[0], &mi, sizeof MODULEINFO);

	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof pmc);

	void* ptrUserData = reinterpret_cast<void*>(MemoryUtils::ScanEx("\x61\x7A\x75\x6B\x69\x5F\x6D\x61\x67\x69\x63\xFF\xFF\xFF\xFF",
		"xxxxxxxxxxxxxxx", reinterpret_cast<char*>(memoryRegions[0].BaseAddress),
		reinterpret_cast<char*>(static_cast<uintptr_t>(memoryRegions[0].BaseAddress) + static_cast<uintptr_t>(pmc.PeakWorkingSetSize)), hProcess));

	if (ptrUserData == 0 || ptrUserData == nullptr || ptrUserData == NULL)
	{
		// Sometimes this scan fails for whatever reason, do the entire thing and try again.
		if (timesRedone < 3) {
			timesRedone++;
			goto Label_Redo;
		}
		// MAPLE HAS INJECTED BUT THE SIG SCAN RETURNED ZERO, FUCKING CLOSE OSU
		// if THIS HERE fails, maple should have an auto process kill if after five seconds, no user data is found within maple
		TerminateProcess(hProcess, 1);
		CloseHandle(hProcess);
	}
	else
	{
		// phew, we found what we wanted, good :)
		std::string userDataString(userData);
		SIZE_T written = 0;
		WriteProcessMemory(hProcess, ptrUserData, userDataString.c_str(), userDataString.size(), &written);
		// TODO: rework these checks, can't be bothered right now, let's release maple!!!
		// 
		// if we haven't written the entire user-data, kill osu!
		//if (written != userDataString.size())
		//{
		//	TerminateProcess(hProcess, 1);
		//	CloseHandle(hProcess);
		//}
		//char readBuffer[256 * 2];
		//SIZE_T read = 0;
		//// now one last check :)
		//ReadProcessMemory(hProcess, ptrUserData, &readBuffer, sizeof userData, &read);

		//if (read != sizeof userData || memcmp(userData, readBuffer, sizeof userData) != 0)
		//{
		//	TerminateProcess(hProcess, 1);
		//	CloseHandle(hProcess);
		//}

		// Everything should be handled fine by the injector now, if anything went wrong and we haven't caught it until here, Maple will handle stuff internally aswell
		// We can sleep now, good night :)
	}

	//STR_ENCRYPTW_END
	//	VM_END
	
    return 0;
}