#pragma once

enum class CheatStatus : uint32_t
{
	Undetected = 0,
	Outdated = 1,
	Detected = 2,
	Unknown = 3
};