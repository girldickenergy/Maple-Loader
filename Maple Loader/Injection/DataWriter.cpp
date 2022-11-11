#include "DataWriter.h"

#include <windows.h>
#include <vector>

unsigned char DataWriter::charToByte(char ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';

	if (ch >= 'a' && ch <= 'f')
		return ch - 'a' + 10;

	if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;

	return 0xCC;
}

unsigned char DataWriter::stichByte(char a, char b)
{
	return charToByte(a) << 4 | charToByte(b);
}

uintptr_t DataWriter::findDataPointer()
{
	std::string pattern = "61 7A 75 6B 69 6D 61 67 69 63";
	std::vector<unsigned char> sig;
	for (unsigned int i = 0u; i < pattern.size(); i++)
	{
		const char& ch = pattern[i];

		if (ch == '?')
		{
			if (i > 0 && pattern[i - 1] != '?')
				sig.push_back(0xCC);
		}
		else if (std::isxdigit(ch))
			sig.push_back(stichByte(ch, pattern[++i]));
	}

	MEMORY_BASIC_INFORMATION mbi;
	PVOID address = nullptr;

	while (VirtualQueryEx(hProcess, address, reinterpret_cast<PMEMORY_BASIC_INFORMATION>(&mbi), sizeof mbi) != 0)
	{
		if (mbi.State == MEM_COMMIT && mbi.Protect != PAGE_NOACCESS && !(mbi.Protect & PAGE_GUARD))
		{
			DWORD previousProtect;
			if (VirtualProtectEx(hProcess, mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &previousProtect))
			{
				unsigned char* buffer = new unsigned char[mbi.RegionSize];
				size_t bytesRead;
				ReadProcessMemory(hProcess, mbi.BaseAddress, buffer, mbi.RegionSize, &bytesRead);
				VirtualProtectEx(hProcess, mbi.BaseAddress, mbi.RegionSize, previousProtect, &previousProtect);

				for (unsigned int mem = 0; mem < bytesRead - sig.size(); mem++)
				{
					bool found = true;

					for (unsigned int i = 0; i < sig.size(); i++)
					{
						if (sig[i] != 0xCC && sig[i] != buffer[mem + i])
						{
							found = false;

							break;
						}
					}

					if (found)
						return (uintptr_t)mbi.BaseAddress + mem;
				}

				delete[] buffer;
			}
		}

		address = reinterpret_cast<PVOID>((uintptr_t)mbi.BaseAddress + mbi.RegionSize);
	}

	return 0u;
}

bool DataWriter::Initialize(DWORD processID)
{
	if (!processID)
		return false;

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
	if (!hProcess)
		return false;

	if (dataPointer = findDataPointer())
		return true;

	CloseHandle(hProcess);
	TerminateProcess(hProcess, -1);

	return false;
}

void DataWriter::Finish()
{
	unsigned int signature = 0xdeadbeef;

	WriteProcessMemory(hProcess, (LPVOID)dataPointer, &signature, sizeof(unsigned int), NULL);

	CloseHandle(hProcess);
}

bool DataWriter::WriteUserData(const std::string& username, const std::string& sessionToken, const std::string& discordID, const std::string& discordAvatarHash, unsigned int cheatID, const std::string& releaseStream)
{
	UserInfoStruct userInfoStruct = UserInfoStruct();
	sprintf(userInfoStruct.Username, "%s", username.c_str());
	sprintf(userInfoStruct.SessionToken, "%s", sessionToken.c_str());
	sprintf(userInfoStruct.DiscordID, "%s", discordID.c_str());
	sprintf(userInfoStruct.DiscordAvatarHash, "%s", discordAvatarHash.c_str());
	sprintf(userInfoStruct.ReleaseStream, "%s", releaseStream.c_str());
	userInfoStruct.CheatID = cheatID;

	if (WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(dataPointer + sizeof(unsigned int)), &userInfoStruct, sizeof(UserInfoStruct), NULL))
		return true;

	TerminateProcess(hProcess, -1);
	CloseHandle(hProcess);

	return false;
}

DataWriter* DataWriter::GetInstance()
{
	if (!instance)
		instance = new DataWriter();

	return instance;
}
