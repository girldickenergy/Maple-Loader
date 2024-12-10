#pragma once

#include <vector>

#include "../IPacket.h"

class HandshakeResponse : public IPacket
{
	std::vector<unsigned char> m_Key;
	std::vector<unsigned char> m_EncryptedKey;
	std::vector<unsigned char> m_EncryptedIV;

	static uint32_t GetStaticIdentifier();
public:
	const std::vector<unsigned char>& GetKey();
	const std::vector<unsigned char>& GetEncryptedKey();
	const std::vector<unsigned char>& GetEncryptedIV();

	uint32_t GetIdentifier() override;
	static void Register();
};
