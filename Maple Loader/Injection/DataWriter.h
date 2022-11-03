#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <string>

struct UserInfoStruct
{
public:
	char Username[24];
	char SessionToken[32];
	char DiscordID[32];
	char DiscordAvatarHash[32];
	unsigned int CheatID;
	char ReleaseStream[16];
};

class DataWriter
{
	DataWriter() = default;
	static inline DataWriter* instance = nullptr;

	HANDLE hProcess;
	uintptr_t dataPointer;

	unsigned char charToByte(char ch);
	unsigned char stichByte(char a, char b);
	uintptr_t findDataPointer();
public:
	bool Initialize(DWORD processID);
	bool WriteUserData(const std::string& username, const std::string& sessionToken, const std::string& discordID, const std::string& discordAvatarHash, unsigned int cheatID, const std::string& releaseStream);
	void Finish();

	static DataWriter* GetInstance();
};