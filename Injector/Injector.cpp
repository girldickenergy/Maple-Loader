// Injector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <algorithm>

#include "MemoryUtils.h"
#include "../ThemidaSDK.h"

using pLoadLibraryA = HMODULE(__stdcall*)(LPCSTR);
using pGetProcAddress = FARPROC(__stdcall*)(HMODULE, LPCSTR);

using dllMain = INT(__stdcall*)(HMODULE, DWORD, LPVOID);
#pragma optimize("", off)
struct lData
{
	LPVOID ImageBase;

	PIMAGE_NT_HEADERS NtHeaders;
	PIMAGE_BASE_RELOCATION BaseReloc;
	PIMAGE_IMPORT_DESCRIPTOR ImportDirectory;

	pLoadLibraryA fnLoadLibraryA;
	pGetProcAddress fnGetProcAddress;
};

auto FindProcessId(const std::wstring& processName) -> DWORD
{
	VM_START
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
	VM_END
	return 0;
}

DWORD __stdcall LibraryLoader(LPVOID Memory)
{
	VM_START
	auto* LoaderParams = static_cast<lData*>(Memory);

	PIMAGE_BASE_RELOCATION pIBR = LoaderParams->BaseReloc;

	auto delta = (DWORD)(static_cast<LPBYTE>(LoaderParams->ImageBase) - LoaderParams->NtHeaders->OptionalHeader.
		ImageBase); // Calculate the delta

	while (pIBR->VirtualAddress != 0u)
	{
		if (pIBR->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION))
		{
			int count = (pIBR->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			auto* list = reinterpret_cast<PWORD>(pIBR + 1);

			for (int i = 0; i < count; i++)
			{
				if (list[i] != 0u)
				{
					auto* ptr = reinterpret_cast<PDWORD>(static_cast<LPBYTE>(LoaderParams->ImageBase) + (pIBR->
						VirtualAddress + (list[i]
							& 0xFFF)));
					*ptr += delta;
				}
			}
		}

		pIBR = reinterpret_cast<PIMAGE_BASE_RELOCATION>(reinterpret_cast<LPBYTE>(pIBR) + pIBR->SizeOfBlock);
	}

	PIMAGE_IMPORT_DESCRIPTOR pIID = LoaderParams->ImportDirectory;

	// Resolve DLL imports
	while (pIID->Characteristics != 0u)
	{
		auto* OrigFirstThunk = reinterpret_cast<PIMAGE_THUNK_DATA>(static_cast<LPBYTE>(LoaderParams->ImageBase) + pIID->
			OriginalFirstThunk);
		auto* FirstThunk = reinterpret_cast<PIMAGE_THUNK_DATA>(static_cast<LPBYTE>(LoaderParams->ImageBase) + pIID->
			FirstThunk);

		HMODULE hModule = LoaderParams->fnLoadLibraryA(static_cast<LPCSTR>(LoaderParams->ImageBase) + pIID->Name);

		if (hModule == nullptr)
		{
			return FALSE;
		}

		while (OrigFirstThunk->u1.AddressOfData != 0u)
		{
			if ((OrigFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) != 0u)
			{
				// Import by ordinal
				auto Function = (DWORD)LoaderParams->fnGetProcAddress(hModule,
				                                                      (LPCSTR)(OrigFirstThunk->u1.Ordinal & 0xFFFF));
				if (Function == 0u)
					return FALSE;

				FirstThunk->u1.Function = Function;
			}
			else
			{
				// Import by name
				auto* pIBN = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(static_cast<LPBYTE>(LoaderParams->ImageBase) +
					OrigFirstThunk->u1.
					                AddressOfData);
				auto Function = (DWORD)LoaderParams->fnGetProcAddress(hModule, static_cast<LPCSTR>(pIBN->Name));
				if (Function == 0u)
					return FALSE;

				FirstThunk->u1.Function = Function;
			}
			OrigFirstThunk++;
			FirstThunk++;
		}
		pIID++;
	}

	if (LoaderParams->NtHeaders->OptionalHeader.AddressOfEntryPoint != 0u)
	{
		auto EntryPoint = reinterpret_cast<dllMain>(static_cast<LPBYTE>(LoaderParams->ImageBase) + LoaderParams->
			NtHeaders->
			OptionalHeader.AddressOfEntryPoint);

		return EntryPoint(static_cast<HMODULE>(LoaderParams->ImageBase), DLL_PROCESS_ATTACH, nullptr);
		// Call the entry point
	}
	VM_END
	return TRUE;
}

DWORD __stdcall stub()
{
	return 0;
}

auto main() -> int
{
	VM_START
	STR_ENCRYPTW_START
	unsigned char azukiMagic[] = {0x61, 0x7a, 0x75, 0x6b, 0x69, 0x5f, 0x6d, 0x61, 0x67, 0x69, 0x63};
	unsigned char azukiMagicRev[] = { 0x63, 0x69, 0x67, 0x61, 0x6d, 0x5f, 0x69, 0x6b, 0x75, 0x7a, 0x61 };
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	auto* mapleBinary = static_cast<char*>(malloc(100000000)); // 100 mb
	auto* userData = static_cast<char*>(malloc(256 * 2)); // we don't need this whole address space, but better safe than sorry

	memset(mapleBinary, 0xFF, 100000000); // set entire memory space to 0xFF
	memset(userData, 0xFF, 256*2); // set entire memory space to 0xFF

	memcpy(mapleBinary, azukiMagic, sizeof azukiMagic); // copy "azuki_magic" into mapleBinary region
	memcpy(userData, azukiMagicRev, sizeof azukiMagicRev); // copy "cigam_ikuza" into mapleBinary region

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

	lData LoaderParams{};

	auto* pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(mapleBinary);
	auto* pNtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<LPBYTE>(mapleBinary) + pDosHeader->
		e_lfanew);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);

	auto* pDosHeaderOsu = reinterpret_cast<PIMAGE_DOS_HEADER>(hProcess);
	auto* pNtHeadersOsu = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<LPBYTE>(hProcess) + pDosHeaderOsu->
		e_lfanew);
	
	PVOID ExecutableImage = VirtualAllocEx(hProcess, nullptr, pNtHeaders->OptionalHeader.SizeOfImage,
	                                       MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	WriteProcessMemory(hProcess, ExecutableImage, mapleBinary,
	                   pNtHeaders->OptionalHeader.SizeOfHeaders, nullptr);

	auto* pSectHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(pNtHeaders + 1);
	for (int i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++)
	{
		WriteProcessMemory(hProcess, static_cast<LPBYTE>(ExecutableImage) + pSectHeader[i].VirtualAddress,
		                   reinterpret_cast<LPBYTE>(mapleBinary) + pSectHeader[i].PointerToRawData,
		                   pSectHeader[i].SizeOfRawData,
		                   nullptr);
	}

	PVOID LoaderMemory = VirtualAllocEx(hProcess, nullptr, 4096, MEM_COMMIT | MEM_RESERVE,
	                                    PAGE_EXECUTE_READWRITE);

	LoaderParams.ImageBase = ExecutableImage;
	LoaderParams.NtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(static_cast<LPBYTE>(ExecutableImage) + pDosHeader->
		e_lfanew);

	LoaderParams.BaseReloc = reinterpret_cast<PIMAGE_BASE_RELOCATION>(static_cast<LPBYTE>(ExecutableImage)
		+ pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
	LoaderParams.ImportDirectory = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(static_cast<LPBYTE>(ExecutableImage)
		+ pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	LoaderParams.fnLoadLibraryA = LoadLibraryA;
	LoaderParams.fnGetProcAddress = GetProcAddress;

	WriteProcessMemory(hProcess, LoaderMemory, &LoaderParams, sizeof(lData),
	                   nullptr);
	WriteProcessMemory(hProcess, static_cast<lData*>(LoaderMemory) + 1, LibraryLoader,
	                   (DWORD)stub - (DWORD)LibraryLoader, nullptr);
	HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0,
	                                    reinterpret_cast<LPTHREAD_START_ROUTINE>(static_cast<lData*>(LoaderMemory) + 1),
	                                    LoaderMemory, 0, nullptr);

	WaitForSingleObject(hThread, INFINITE);

	VirtualFreeEx(hProcess, LoaderMemory, 0, MEM_RELEASE);
	
	void* ptrUserData = reinterpret_cast<void*>(MemoryUtils::FindSignature("\x61\x7A\x75\x6B\x69\x5F\x6D\x61\x67\x69\x63\xFF\xFF\xFF\xFF",
	                                                                       "xxxxxxxxxxxxxxx", (uintptr_t)hProcess,
	                                                                       pNtHeadersOsu->OptionalHeader.SizeOfImage));

	if (ptrUserData == 0 ||ptrUserData == nullptr || ptrUserData == NULL)
	{
		// MAPLE HAS INJECTED BUT THE SIG SCAN RETURNED ZERO, FUCKING CLOSE OSU
		// if THIS HERE fails, maple should have an auto process kill if after five seconds, no user data is found within maple
		TerminateProcess(hProcess, 1);
		CloseHandle(hProcess);
	}
	else
	{
		// phew, we found what we wanted, good :)
		SIZE_T written = 0;
		WriteProcessMemory(hProcess, ptrUserData, userData, sizeof userData, &written);
		// if we haven't written the entire user-data, kill osu!
		if (written != sizeof userData) 
		{
			TerminateProcess(hProcess, 1);
			CloseHandle(hProcess);
		}
		char readBuffer[256*2];
		SIZE_T read = 0;
		// now one last check :)
		ReadProcessMemory(hProcess, ptrUserData, &readBuffer, sizeof userData, &read);
		
		if (read != sizeof userData || memcmp(userData, readBuffer, sizeof userData) != 0)
		{
			TerminateProcess(hProcess, 1);
			CloseHandle(hProcess);
		}

		// Everything should be handled fine by the injector now, if anything went wrong and we haven't caught it until here, Maple will handle stuff internally aswell
		// We can sleep now, good night :)
	}
	
	STR_ENCRYPTW_END
	VM_END
}
#pragma optimize("", on)
