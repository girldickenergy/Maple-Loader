// Injector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
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

DWORD FindProcessId(std::wstring processName)
{
	VM_START
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	Process32First(processSnapshot, &processInfo);
	if (!processName.compare(processInfo.szExeFile))
	{
		CloseHandle(processSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(processSnapshot, &processInfo))
	{
		if (!processName.compare(processInfo.szExeFile))
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
	auto LoaderParams = static_cast<lData*>(Memory);

	PIMAGE_BASE_RELOCATION pIBR = LoaderParams->BaseReloc;

	DWORD delta = (DWORD)(static_cast<LPBYTE>(LoaderParams->ImageBase) - LoaderParams->NtHeaders->OptionalHeader.
		ImageBase); // Calculate the delta

	while (pIBR->VirtualAddress)
	{
		if (pIBR->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION))
		{
			int count = (pIBR->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			auto list = (PWORD)(pIBR + 1);

			for (int i = 0; i < count; i++)
			{
				if (list[i])
				{
					auto ptr = (PDWORD)(static_cast<LPBYTE>(LoaderParams->ImageBase) + (pIBR->VirtualAddress + (list[i]
						& 0xFFF)));
					*ptr += delta;
				}
			}
		}

		pIBR = (PIMAGE_BASE_RELOCATION)((LPBYTE)pIBR + pIBR->SizeOfBlock);
	}

	PIMAGE_IMPORT_DESCRIPTOR pIID = LoaderParams->ImportDirectory;

	// Resolve DLL imports
	while (pIID->Characteristics)
	{
		auto OrigFirstThunk = (PIMAGE_THUNK_DATA)(static_cast<LPBYTE>(LoaderParams->ImageBase) + pIID->
			OriginalFirstThunk);
		auto FirstThunk = (PIMAGE_THUNK_DATA)(static_cast<LPBYTE>(LoaderParams->ImageBase) + pIID->FirstThunk);

		HMODULE hModule = LoaderParams->fnLoadLibraryA(static_cast<LPCSTR>(LoaderParams->ImageBase) + pIID->Name);

		if (!hModule)
			return FALSE;

		while (OrigFirstThunk->u1.AddressOfData)
		{
			if (OrigFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
			{
				// Import by ordinal
				DWORD Function = (DWORD)LoaderParams->fnGetProcAddress(hModule,
				                                                       (LPCSTR)(OrigFirstThunk->u1.Ordinal & 0xFFFF));

				if (!Function)
					return FALSE;

				FirstThunk->u1.Function = Function;
			}
			else
			{
				// Import by name
				auto pIBN = (PIMAGE_IMPORT_BY_NAME)(static_cast<LPBYTE>(LoaderParams->ImageBase) + OrigFirstThunk->u1.
					AddressOfData);
				DWORD Function = (DWORD)LoaderParams->fnGetProcAddress(hModule, static_cast<LPCSTR>(pIBN->Name));
				if (!Function)
					return FALSE;

				FirstThunk->u1.Function = Function;
			}
			OrigFirstThunk++;
			FirstThunk++;
		}
		pIID++;
	}

	if (LoaderParams->NtHeaders->OptionalHeader.AddressOfEntryPoint)
	{
		auto EntryPoint = (dllMain)(static_cast<LPBYTE>(LoaderParams->ImageBase) + LoaderParams->NtHeaders->
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

int main()
{
	VM_START
	unsigned char azukiMagic[] = {0x61, 0x7a, 0x75, 0x6b, 0x69, 0x5f, 0x6d, 0x61, 0x67, 0x69, 0x63};
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	auto mapleBinary = static_cast<char*>(malloc(100000000)); // 100 mb

	memset(mapleBinary, 0xFF, 100000000); // set entire memory space to 0xFF

	memcpy(mapleBinary, azukiMagic, sizeof azukiMagic); // copy "azuki_magic" into mapleBinary region

	while (memcmp(mapleBinary, azukiMagic, sizeof azukiMagic) == 0)
	{
		std::cout << "non written" << std::endl;
		Sleep(1500); // don't know how fast memory writes will happen so ye
	}

	DWORD ProcessId = FindProcessId(L"osu!.exe");

	lData LoaderParams;

	auto pDosHeader = (PIMAGE_DOS_HEADER)mapleBinary;
	auto pNtHeaders = (PIMAGE_NT_HEADERS)((LPBYTE)mapleBinary + pDosHeader->e_lfanew);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);
	PVOID ExecutableImage = VirtualAllocEx(hProcess, nullptr, pNtHeaders->OptionalHeader.SizeOfImage,
	                                       MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	WriteProcessMemory(hProcess, ExecutableImage, mapleBinary,
	                   pNtHeaders->OptionalHeader.SizeOfHeaders, nullptr);

	auto pSectHeader = (PIMAGE_SECTION_HEADER)(pNtHeaders + 1);
	for (int i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++)
		WriteProcessMemory(hProcess, static_cast<LPBYTE>(ExecutableImage) + pSectHeader[i].VirtualAddress,
		                   (LPBYTE)mapleBinary + pSectHeader[i].PointerToRawData, pSectHeader[i].SizeOfRawData,
		                   nullptr);

	PVOID LoaderMemory = VirtualAllocEx(hProcess, nullptr, 4096, MEM_COMMIT | MEM_RESERVE,
	                                    PAGE_EXECUTE_READWRITE);

	LoaderParams.ImageBase = ExecutableImage;
	LoaderParams.NtHeaders = (PIMAGE_NT_HEADERS)(static_cast<LPBYTE>(ExecutableImage) + pDosHeader->e_lfanew);

	LoaderParams.BaseReloc = (PIMAGE_BASE_RELOCATION)(static_cast<LPBYTE>(ExecutableImage)
		+ pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
	LoaderParams.ImportDirectory = (PIMAGE_IMPORT_DESCRIPTOR)(static_cast<LPBYTE>(ExecutableImage)
		+ pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	LoaderParams.fnLoadLibraryA = LoadLibraryA;
	LoaderParams.fnGetProcAddress = GetProcAddress;

	WriteProcessMemory(hProcess, LoaderMemory, &LoaderParams, sizeof(lData),
	                   nullptr);
	WriteProcessMemory(hProcess, static_cast<lData*>(LoaderMemory) + 1, LibraryLoader,
	                   (DWORD)stub - (DWORD)LibraryLoader, nullptr);
	HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0,
	                                    (LPTHREAD_START_ROUTINE)(static_cast<lData*>(LoaderMemory) + 1),
	                                    LoaderMemory, 0, nullptr);

	std::cout << "Address of Loader: " << std::hex << LoaderMemory << std::endl;
	std::cout << "Address of Image: " << std::hex << ExecutableImage << std::endl;

	WaitForSingleObject(hThread, INFINITE);

	VirtualFreeEx(hProcess, LoaderMemory, 0, MEM_RELEASE);
	VM_END
}
#pragma optimize("", on)