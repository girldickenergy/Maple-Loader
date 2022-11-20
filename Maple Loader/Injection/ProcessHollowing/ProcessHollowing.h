#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct ProcessAddressInformation
{
	LPVOID lpProcessPEBAddress;
	LPVOID lpProcessImageBaseAddress;
};

typedef struct IMAGE_RELOCATION_ENTRY
{
	WORD Offset : 12;
	WORD Type : 4;
} IMAGE_RELOCATION_ENTRY, * PIMAGE_RELOCATION_ENTRY;

class ProcessHollowing
{
	static HANDLE getFileContent(const wchar_t* lpFilePath);
	static BOOL isValidPE(const LPVOID lpImage);
	static BOOL isPE32(const LPVOID lpImage);
	static ProcessAddressInformation getProcessAddressInformation32(const PPROCESS_INFORMATION lpPI);
	static ProcessAddressInformation getProcessAddressInformation64(const PPROCESS_INFORMATION lpPI);
	static DWORD getSubsytem32(const LPVOID lpImage);
	static DWORD getSubsytem64(const LPVOID lpImage);
	static DWORD getSubsystemEx32(const HANDLE hProcess, const LPVOID lpImageBaseAddress);
	static DWORD getSubsystemEx64(const HANDLE hProcess, const LPVOID lpImageBaseAddress);
	static void cleanAndExitProcess(const LPPROCESS_INFORMATION lpPI, const HANDLE hFileContent);
	static void cleanProcess(const LPPROCESS_INFORMATION lpPI, const HANDLE hFileContent);
	static BOOL hasRelocation32(const LPVOID lpImage);
	static BOOL hasRelocation64(const LPVOID lpImage);
	static IMAGE_DATA_DIRECTORY getRelocAddress32(const LPVOID lpImage);
	static IMAGE_DATA_DIRECTORY getRelocAddress64(const LPVOID lpImage);
	static BOOL runPE32(const LPPROCESS_INFORMATION lpPI, const LPVOID lpImage);
	static BOOL runPE64(const LPPROCESS_INFORMATION lpPI, const LPVOID lpImage);
	static BOOL runPEReloc32(const LPPROCESS_INFORMATION lpPI, const LPVOID lpImage);
	static BOOL runPEReloc64(const LPPROCESS_INFORMATION lpPI, const LPVOID lpImage);
	static DWORD runPE(LPVOID payloadData, const wchar_t* targetPath, bool fromFile = false);
public:
	static DWORD RunPE(const wchar_t* payloadPath, const wchar_t* targetPath32, const wchar_t* targetPath64);
	static DWORD RunPE(LPVOID payloadData, const wchar_t* targetPath32, const wchar_t* targetPath64);
};