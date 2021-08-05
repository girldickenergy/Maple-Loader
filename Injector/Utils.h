#pragma once

#include <memory>
#include <stdexcept>
#include <Windows.h>
#include <winternl.h>

#include "Process.h"

struct SYSTEM_THREADS
{
    LARGE_INTEGER  KernelTime;
    LARGE_INTEGER  UserTime;
    LARGE_INTEGER  CreateTime;
    ULONG          WaitTime;
    PVOID          StartAddress;
    CLIENT_ID      ClientId;
    KPRIORITY      Priority;
    KPRIORITY      BasePriority;
    ULONG          ContextSwitchCount;
    LONG           State;
    LONG           WaitReason;
};

struct VM_COUNTERS
{
    SIZE_T             PeakVirtualSize;
    SIZE_T             VirtualSize;
    ULONG              PageFaultCount;
    SIZE_T             PeakWorkingSetSize;
    SIZE_T             WorkingSetSize;
    SIZE_T             QuotaPeakPagedPoolUsage;
    SIZE_T             QuotaPagedPoolUsage;
    SIZE_T             QuotaPeakNonPagedPoolUsage;
    SIZE_T             QuotaNonPagedPoolUsage;
    SIZE_T             PagefileUsage;
    SIZE_T             PeakPagefileUsage;
};

struct SYSTEM_PROCESSES
{
    ULONG              NextEntryDelta;
    ULONG              ThreadCount;
    ULONG              Reserved1[6];
    LARGE_INTEGER      CreateTime;
    LARGE_INTEGER      UserTime;
    LARGE_INTEGER      KernelTime;
    UNICODE_STRING     ProcessName;
    KPRIORITY          BasePriority;
    ULONG              ProcessId;
    ULONG              InheritedFromProcessId;
    ULONG              HandleCount;
    ULONG              Reserved2[2];
    VM_COUNTERS        VmCounters;
    IO_COUNTERS        IoCounters;
    SYSTEM_THREADS     Threads[1];
};

class Utils
{
    using f_NTQuerySystemInformation = NTSTATUS(WINAPI*)(ULONG, PVOID, ULONG, PULONG);
public:
    static Process GetProcess(std::wstring_view name)
    {
        const auto hNTDLL{ LoadLibraryW(L"ntdll") };
        if (!hNTDLL)
            std::throw_with_nested(std::runtime_error("Unable to get handle of ntdll.dll library"));

        const auto ntQSI{ reinterpret_cast<f_NTQuerySystemInformation>(GetProcAddress(hNTDLL, "NtQuerySystemInformation")) };
        if (!ntQSI)
            std::throw_with_nested(std::runtime_error("Unable to load NTQuerySystemInformation function"));

        if (SIZE_T buffersize; !NT_SUCCESS(ntQSI(SystemProcessInformation, nullptr, 0, &buffersize)))
        {
            auto buffer{ std::make_unique<std::byte[]>(buffersize) };
            auto spi{ reinterpret_cast<SYSTEM_PROCESSES*>(buffer.get()) };
            if (NTSTATUS status; !NT_SUCCESS(status = ntQSI(SystemProcessInformation, spi, buffersize, nullptr)))
                std::throw_with_nested(std::runtime_error("Error: Unable to query list of running processes: " + status));

            while (spi->NextEntryDelta)
            {
                spi = reinterpret_cast<SYSTEM_PROCESSES*>((LPBYTE)spi + spi->NextEntryDelta);
                const auto processName{ spi->ProcessName.Buffer };
            	if (processName != name)
                    continue;
                const auto pid{ spi->ProcessId };
                const auto tid{ reinterpret_cast<ULONG>(spi->Threads->ClientId.UniqueThread) };
                return Process { processName, pid, tid };
            }
        }
    }
};