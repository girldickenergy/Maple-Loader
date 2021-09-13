#pragma once

#include <string>
#include <vector>
#include <windows.h>

#include "hde32/hde32.h"

#include "../../ThemidaSDK.h"

class Azuki
{
	static inline std::vector<std::tuple<uintptr_t, std::vector<unsigned char>>> MonitoredSubroutines;

	static void MonitorSubroutine(std::string module, std::string func);
	static void MonitorSubroutine(DWORD* address, std::string func);
public:
	static DWORD* GetPointer(std::string module, std::string func);
	static void Install();
};