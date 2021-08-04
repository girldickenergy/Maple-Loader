#pragma once

#include <vector>
#include <windows.h>

struct MemoryRegion
{
	DWORD BaseAddress;
	SIZE_T RegionSize;
	DWORD State;
	DWORD Protect;
	DWORD Type;

	explicit MemoryRegion(MEMORY_BASIC_INFORMATION32 mbi)
	{
		BaseAddress = mbi.BaseAddress;
		RegionSize = mbi.RegionSize;
		State = mbi.State;
		Protect = mbi.Protect;
		Type = mbi.Type;
	}
};

class MemoryUtils
{
	static inline std::vector<MemoryRegion> memoryRegions;

	static void cacheMemoryRegions()
	{
		memoryRegions.clear();

		MEMORY_BASIC_INFORMATION32 mbi;
		LPCVOID address = nullptr;

		while (VirtualQueryEx(GetCurrentProcess(), address, reinterpret_cast<PMEMORY_BASIC_INFORMATION>(&mbi),
		                      sizeof mbi) != 0)
		{
			if (mbi.State == MEM_COMMIT && mbi.Protect >= 0x10 && mbi.Protect <= 0x80)
			{
				memoryRegions.push_back(*new MemoryRegion(mbi));
			}
			address = reinterpret_cast<LPCVOID>(mbi.BaseAddress + mbi.RegionSize);
		}
	}

public:
	static auto FindSignature(const char* pattern, const char* mask) -> uintptr_t
	{
		if (memoryRegions.empty())
		{
			cacheMemoryRegions();
		}

		for (const auto& region : memoryRegions)
		{
			const size_t patternLength = strlen(mask);

			for (uintptr_t i = 0; i < region.RegionSize - patternLength; i++)
			{
				bool found = true;
				for (uintptr_t j = 0; j < patternLength; j++)
				{
					if (mask[j] != '?' && pattern[j] != *(char*)(region.BaseAddress + i + j))
					{
						found = false;
						break;
					}
				}

				if (found)
				{
					return region.BaseAddress + i;
				}
			}
		}

		return NULL;
	}

	static auto FindSignature(const char* signature, const char* mask, uintptr_t entryPoint, int size) -> uintptr_t
	{
		const size_t signatureLength = strlen(mask);

		for (uintptr_t i = 0; i < size - signatureLength; i++)
		{
			bool found = true;
			for (uintptr_t j = 0; j < signatureLength; j++)
			{
				if (mask[j] != '?' && signature[j] != *(char*)(entryPoint + i + j))
				{
					found = false;
					break;
				}
			}

			if (found)
			{
				return entryPoint + i;
			}
		}

		return NULL;
	}

	static auto GetModuleSize(const char* moduleName) -> DWORD
	{
		const HMODULE module = GetModuleHandleA(moduleName);
		if (module == nullptr)
		{
			return 0;
		}

		auto pDOSHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(module);
		auto pNTHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<BYTE*>(pDOSHeader) + pDOSHeader->
			e_lfanew);

		return pNTHeaders->OptionalHeader.SizeOfImage;
	}
};