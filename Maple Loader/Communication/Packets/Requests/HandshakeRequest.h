#pragma once

#include <vector>

#include "../IPacket.h"

class HandshakeRequest : public IPacket
{
	std::vector<uint32_t> m_RandomJunk;

	static uint32_t GetStaticIdentifier();
public:
	HandshakeRequest(const std::vector<uint32_t>& randomJunk);

	uint32_t GetIdentifier() override;
	static void Register();
};
