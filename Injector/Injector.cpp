#include <Windows.h>
#include <TlHelp32.h>
#include <algorithm>
#include <psapi.h>
#include <filesystem>

#include "MemoryUtils.h"

#include "blackbone/BlackBone/Process/Process.h"

#include <ThemidaSDK.h>

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
	VM_EAGLE_BLACK_START
	unsigned char azukiMagic[] = { 0x61, 0x7a, 0x75, 0x6b, 0x69, 0x5f, 0x6d, 0x61, 0x67, 0x69, 0x63 };
	unsigned char azukiMagicRev[] = { 0x63, 0x69, 0x67, 0x61, 0x6d, 0x5f, 0x69, 0x6b, 0x75, 0x7a, 0x61 };

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

	HANDLE mtx = CreateMutexA(NULL, FALSE, "QVPj0LSOL81Lko4d");

	// Synchronize with loader
	while (*((DWORD*)mapleBinary) != 0x13371337)
		Sleep(50);

	CloseHandle(mtx);

	// Skip over synchronization primitive
	mapleBinary += 4;

	const auto oldNtHeader{ reinterpret_cast<IMAGE_NT_HEADERS*>(mapleBinary + reinterpret_cast<IMAGE_DOS_HEADER*>(mapleBinary)->e_lfanew) };

	adjustPrivileges();
	DWORD osu = FindProcessId(L"osu!.exe");
	while (osu == 0)
	{
		osu = FindProcessId(L"osu!.exe");
		Sleep(1000);
	}
	Sleep(5000);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, osu);

	blackbone::Process proc;
	proc.Attach(osu);

	// Create a structure to hold our loader data
	struct DllArgs
	{
		char user_data[256 * 5 * 10];
	} dll_args;

	// Initialize loader data.
	memset(dll_args.user_data, 0x00, 256 * 5 * 10);
	std::string userDataString(userData);
	memcpy(dll_args.user_data, userDataString.c_str(), userDataString.size());

	// Setup custom args structure for blackbone
	blackbone::CustomArgs_t args;
	args.push_back(&dll_args);

	// Finally, inject the dll.
	auto image = proc.mmap().MapImage(oldNtHeader->OptionalHeader.SizeOfImage, mapleBinary, false,
		blackbone::RebaseProcess | blackbone::NoDelayLoad | blackbone::WipeHeader,
		nullptr, nullptr, &args);
	VM_EAGLE_BLACK_END

	return 0;
}