#pragma once

#include <Windows.h>
#include <vector>
#include <string>
#include <stdexcept>

#include "Process.h"

using _LoadLibraryA = HMODULE(__stdcall*)(const char*);
using _GetProcAddress = FARPROC(__stdcall*)(HMODULE, const char*);
using _DLL_ENTRY_POINT = BOOL(__stdcall*)(void* dll, unsigned long reason, void* reserved);

struct MANUAL_MAPPING_DATA
{
    _LoadLibraryA		LoadLibraryA;
    _GetProcAddress	GetProcAddress;
};

class MMap
{
    MANUAL_MAPPING_DATA data{ LoadLibraryA, GetProcAddress };
    HANDLE process{ nullptr };
    HANDLE thread{ nullptr };

    std::vector<unsigned char> file;
    unsigned char* srcData{ nullptr };
    unsigned char* targetBase{ nullptr };
    void* loader{ nullptr };
    void* codeCave{ nullptr };

    bool threadHijack();

public:
    explicit MMap(Process process, std::vector<unsigned char> _file);

    bool run();

    ~MMap()
    {
        if (thread)
            CloseHandle(thread);
        if (codeCave)
            VirtualFreeEx(process, codeCave, 0, MEM_RELEASE);
        if (loader)
            VirtualFreeEx(process, loader, 0, MEM_RELEASE);
        if (process)
            CloseHandle(process);
    }
};