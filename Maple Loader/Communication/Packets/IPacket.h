#pragma once

#include <cstdint>

class IPacket
{
public:
	virtual uint32_t GetIdentifier() = 0;
};
