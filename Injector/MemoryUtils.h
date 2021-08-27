#pragma once

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
public:
	/**
	 * Returns all the memory regions
	 *
	 * @param hProcess Handle to the process
	 */
	static std::vector<MemoryRegion> GetMemoryRegions(HANDLE hProcess)
	{
		std::vector<MemoryRegion> regions;

		MEMORY_BASIC_INFORMATION32 mbi;
		LPCVOID address = nullptr;

		while (VirtualQueryEx(hProcess, address, reinterpret_cast<PMEMORY_BASIC_INFORMATION>(&mbi), sizeof mbi) != 0)
		{
			if (mbi.State == MEM_COMMIT && mbi.Protect >= 0x10 && mbi.Protect <= 0x80)
			{
				regions.emplace_back(mbi);
			}
			address = reinterpret_cast<LPCVOID>(mbi.BaseAddress + mbi.RegionSize);
		}

		return regions;
	}

	/**
	*Scan the process space for an Array of bytes
	*
	* @param pattern The signature / Array of bytes in string form
	* @param mask The mask of the signature. ? ->wildcard | x -> static byte
	* @param begin The address from where to begin searching
	* @param size Total size to search in
	* @return Address of Array of bytes
	*/
	static auto ScanIn(const char* pattern, const char* mask, char* begin, unsigned int size) -> char*
	{
		const unsigned int patternLength = strlen(mask);

		for (unsigned int i = 0; i < size - patternLength; i++)
		{
			bool found = true;
			for (unsigned int j = 0; j < patternLength; j++)
			{
				if (mask[j] != '?' && pattern[j] != *(begin + i + j))
				{
					found = false;
					break;
				}
			}
			if (found)
			{
				return (begin + i);
			}
		}
		return nullptr;
	}

	/**
	 * External wrapper for the 'ScanIn' function
	 *
	 * @param pattern The signature/Array of bytes in string form
	 * @param mask The mask of the signature. ? -> wildcard | x -> static byte
	 * @param begin The address from where to begin searching
	 * @param end The address where searching will end
	 * @param hProc Handle to the process
	 * @return Address of Array of bytes
	 */
	static auto ScanEx(const char* pattern, const char* mask, char* begin, char* end, HANDLE hProc) -> char*
	{
		char* currentChunk = begin;
		char* match = nullptr;
		SIZE_T bytesRead;

		while (currentChunk < end)
		{
			MEMORY_BASIC_INFORMATION mbi;

			if (!VirtualQueryEx(hProc, currentChunk, &mbi, sizeof(mbi)))
			{
				int err = GetLastError();
				return nullptr;
			}

			char* buffer = new char[mbi.RegionSize];

			if (mbi.State == MEM_COMMIT && mbi.Protect != PAGE_NOACCESS)
			{
				DWORD previousProtect;
				if (VirtualProtectEx(hProc, mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &previousProtect))
				{
					ReadProcessMemory(hProc, mbi.BaseAddress, buffer, mbi.RegionSize, &bytesRead);
					VirtualProtectEx(hProc, mbi.BaseAddress, mbi.RegionSize, previousProtect, &previousProtect);

					if (char* internalAddress = ScanIn(pattern, mask, buffer, bytesRead); internalAddress != nullptr)
					{
						const uintptr_t offsetFromBuffer = internalAddress - buffer;
						match = currentChunk + offsetFromBuffer;
						delete[] buffer;
						break;
					}
				}
			}

			currentChunk = currentChunk + mbi.RegionSize;
			delete[] buffer;
		}
		return match;
	}
};