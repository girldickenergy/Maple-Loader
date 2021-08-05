// Injector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <algorithm>
#include <psapi.h>
#include <filesystem>

#include "MemoryUtils.h"
#include "../ThemidaSDK.h"
typedef HMODULE(WINAPI* pLoadLibraryA)(LPCSTR);
typedef FARPROC(WINAPI* pGetProcAddress)(HMODULE, LPCSTR);

typedef BOOL(WINAPI* PDLL_MAIN)(HMODULE, DWORD, PVOID);

typedef struct _MANUAL_INJECT
{
	PVOID ImageBase;
	PIMAGE_NT_HEADERS NtHeaders;
	PIMAGE_BASE_RELOCATION BaseRelocation;
	PIMAGE_IMPORT_DESCRIPTOR ImportDirectory;
	pLoadLibraryA fnLoadLibraryA;
	pGetProcAddress fnGetProcAddress;
}MANUAL_INJECT, * PMANUAL_INJECT;
#pragma optimize("", off)

auto FindProcessId(const std::wstring& processName) -> DWORD
{
	//VM_START
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
	//VM_END
		return 0;
}


DWORD WINAPI LoadDll(PVOID p)
{
	PMANUAL_INJECT ManualInject;

	HMODULE hModule;
	DWORD i, Function, count, delta;

	PDWORD ptr;
	PWORD list;

	PIMAGE_BASE_RELOCATION pIBR;
	PIMAGE_IMPORT_DESCRIPTOR pIID;
	PIMAGE_IMPORT_BY_NAME pIBN;
	PIMAGE_THUNK_DATA FirstThunk, OrigFirstThunk;

	PDLL_MAIN EntryPoint;

	ManualInject = (PMANUAL_INJECT)p;

	pIBR = ManualInject->BaseRelocation;
	delta = (DWORD)((LPBYTE)ManualInject->ImageBase - ManualInject->NtHeaders->OptionalHeader.ImageBase); // Calculate the delta

	// Relocate the image

	while (pIBR->VirtualAddress)
	{
		if (pIBR->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION))
		{
			count = (pIBR->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			list = (PWORD)(pIBR + 1);

			for (i = 0;i < count;i++)
			{
				if (list[i])
				{
					ptr = (PDWORD)((LPBYTE)ManualInject->ImageBase + (pIBR->VirtualAddress + (list[i] & 0xFFF)));
					*ptr += delta;
				}
			}
		}

		pIBR = (PIMAGE_BASE_RELOCATION)((LPBYTE)pIBR + pIBR->SizeOfBlock);
	}

	pIID = ManualInject->ImportDirectory;

	// Resolve DLL imports

	while (pIID->Characteristics)
	{
		OrigFirstThunk = (PIMAGE_THUNK_DATA)((LPBYTE)ManualInject->ImageBase + pIID->OriginalFirstThunk);
		FirstThunk = (PIMAGE_THUNK_DATA)((LPBYTE)ManualInject->ImageBase + pIID->FirstThunk);

		hModule = ManualInject->fnLoadLibraryA((LPCSTR)ManualInject->ImageBase + pIID->Name);

		if (!hModule)
		{
			return FALSE;
		}

		while (OrigFirstThunk->u1.AddressOfData)
		{
			if (OrigFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
			{
				// Import by ordinal

				Function = (DWORD)ManualInject->fnGetProcAddress(hModule, (LPCSTR)(OrigFirstThunk->u1.Ordinal & 0xFFFF));

				if (!Function)
				{
					return FALSE;
				}

				FirstThunk->u1.Function = Function;
			}

			else
			{
				// Import by name

				pIBN = (PIMAGE_IMPORT_BY_NAME)((LPBYTE)ManualInject->ImageBase + OrigFirstThunk->u1.AddressOfData);
				Function = (DWORD)ManualInject->fnGetProcAddress(hModule, (LPCSTR)pIBN->Name);

				if (!Function)
				{
					return FALSE;
				}

				FirstThunk->u1.Function = Function;
			}

			OrigFirstThunk++;
			FirstThunk++;
		}

		pIID++;
	}

	if (ManualInject->NtHeaders->OptionalHeader.AddressOfEntryPoint)
	{
		EntryPoint = (PDLL_MAIN)((LPBYTE)ManualInject->ImageBase + ManualInject->NtHeaders->OptionalHeader.AddressOfEntryPoint);
		return EntryPoint((HMODULE)ManualInject->ImageBase, DLL_PROCESS_ATTACH, NULL); // Call the entry point
	}

	return TRUE;
}

DWORD WINAPI LoadDllEnd()
{
	return 0;
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

auto main() -> int
{
	TOKEN_PRIVILEGES tp;
	MANUAL_INJECT ManualInject;
	//VM_START
	//	STR_ENCRYPTW_START
		unsigned char azukiMagic[] = { 0x61, 0x7a, 0x75, 0x6b, 0x69, 0x5f, 0x6d, 0x61, 0x67, 0x69, 0x63 };
	unsigned char azukiMagicRev[] = { 0x63, 0x69, 0x67, 0x61, 0x6d, 0x5f, 0x69, 0x6b, 0x75, 0x7a, 0x61 };
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	auto* mapleBinary = new char[150000000];
	auto* userData = new char[256 * 5 * 10];
	
	//auto* mapleBinary = static_cast<char*>(malloc(100000000)); // 100 mb
	//auto* userData = static_cast<char*>(malloc(256 * 5 * 10)); // we don't need this whole address space, but better safe than sorry

	memset(mapleBinary, 0xFF, 150000000); // set entire memory space to 0xFF
	memset(userData, 0xFF, 256 * 5 * 10); // set entire memory space to 0xFF

	memcpy(mapleBinary, azukiMagic, sizeof azukiMagic); // copy "azuki_magic" into mapleBinary region
	memcpy(userData, azukiMagicRev, sizeof azukiMagicRev); // copy "cigam_ikuza" into mapleBinary region

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

	DWORD ProcessId = FindProcessId(L"osu!.exe");
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);
	HMODULE modules[250];
	DWORD cbNeeded = 0;
	EnumProcessModules(hProcess, modules, sizeof(modules), &cbNeeded);

	TCHAR path[MAX_PATH];
	HMODULE moduleOsu = modules[0];

	cacheMemoryRegions(hProcess);
	MODULEINFO mi;
	GetModuleInformation(hProcess, moduleOsu, &mi, sizeof MODULEINFO);

	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)& pmc, sizeof pmc);

	auto* pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(mapleBinary);
	auto* pNtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<LPBYTE>(mapleBinary + pDosHeader->
		e_lfanew));
	PIMAGE_SECTION_HEADER pISH;

	LPVOID image = VirtualAllocEx(hProcess, NULL, pNtHeaders->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE); // Allocate memory for the DLL

	WriteProcessMemory(hProcess, image, mapleBinary, pNtHeaders->OptionalHeader.SizeOfHeaders, NULL);

	pISH = (PIMAGE_SECTION_HEADER)(pNtHeaders + 1);

	// Copy the DLL to target process

	printf("\nCopying sections to target process.\n");

	for (int i = 0;i < pNtHeaders->FileHeader.NumberOfSections;i++)
	{
		WriteProcessMemory(hProcess, (PVOID)((LPBYTE)image + pISH[i].VirtualAddress), (PVOID)((LPBYTE)mapleBinary + pISH[i].PointerToRawData), pISH[i].SizeOfRawData, NULL);
	}

	printf("\nAllocating memory for the loader code.\n");
	LPVOID mem = VirtualAllocEx(hProcess, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE); // Allocate memory for the loader code


	printf("\nLoader code allocated at %#x\n", mem);
	memset(&ManualInject, 0, sizeof(MANUAL_INJECT));

	ManualInject.ImageBase = image;
	ManualInject.NtHeaders = (PIMAGE_NT_HEADERS)((LPBYTE)image + pDosHeader->e_lfanew);
	ManualInject.BaseRelocation = (PIMAGE_BASE_RELOCATION)((LPBYTE)image + pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
	ManualInject.ImportDirectory = (PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)image + pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	ManualInject.fnLoadLibraryA = LoadLibraryA;
	ManualInject.fnGetProcAddress = GetProcAddress;

	WriteProcessMemory(hProcess, mem, &ManualInject, sizeof(MANUAL_INJECT), NULL); // Write the loader information to target process
	WriteProcessMemory(hProcess, (PVOID)((PMANUAL_INJECT)mem + 1), LoadDll, (DWORD)LoadDllEnd - (DWORD)LoadDll, NULL); // Write the loader code to target process

	printf("\nExecuting loader code.\n");
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)((PMANUAL_INJECT)mem + 1), mem, 0, NULL); // Create a remote thread to execute the loader code

	WaitForSingleObject(hThread, INFINITE);
	LPDWORD ExitCode;
	GetExitCodeThread(hThread, ExitCode);

	CloseHandle(hThread);
	VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
	CloseHandle(hProcess);

	/*void* ptrUserData = reinterpret_cast<void*>(MemoryUtils::FindSignature("\x61\x7A\x75\x6B\x69\x5F\x6D\x61\x67\x69\x63\xFF\xFF\xFF\xFF",
		"xxxxxxxxxxxxxxx", (uintptr_t)hProcess,
		pNtHeadersOsu->OptionalHeader.SizeOfImage));*/

	//if (ptrUserData == 0 || ptrUserData == nullptr || ptrUserData == NULL)
	//{
	//	// MAPLE HAS INJECTED BUT THE SIG SCAN RETURNED ZERO, FUCKING CLOSE OSU
	//	// if THIS HERE fails, maple should have an auto process kill if after five seconds, no user data is found within maple
	//	TerminateProcess(hProcess, 1);
	//	CloseHandle(hProcess);
	//}
	//else
	//{
	//	// phew, we found what we wanted, good :)
	//	SIZE_T written = 0;
	//	WriteProcessMemory(hProcess, ptrUserData, userData, sizeof userData, &written);
	//	// if we haven't written the entire user-data, kill osu!
	//	if (written != sizeof userData)
	//	{
	//		TerminateProcess(hProcess, 1);
	//		CloseHandle(hProcess);
	//	}
	//	char readBuffer[256 * 2];
	//	SIZE_T read = 0;
	//	// now one last check :)
	//	ReadProcessMemory(hProcess, ptrUserData, &readBuffer, sizeof userData, &read);

	//	if (read != sizeof userData || memcmp(userData, readBuffer, sizeof userData) != 0)
	//	{
	//		TerminateProcess(hProcess, 1);
	//		CloseHandle(hProcess);
	//	}

	//	// Everything should be handled fine by the injector now, if anything went wrong and we haven't caught it until here, Maple will handle stuff internally aswell
	//	// We can sleep now, good night :)
	//}

	//STR_ENCRYPTW_END
	//	VM_END
}
#pragma optimize("", on)
