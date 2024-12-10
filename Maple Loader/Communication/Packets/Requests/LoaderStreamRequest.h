#pragma once

#include <string>

#include "../IPacket.h"

class LoaderStreamRequest : public IPacket
{
	std::string m_SessionToken;
	unsigned int m_CheatID;
	std::string m_ReleseStream;

	static uint32_t GetStaticIdentifier();
public:
	LoaderStreamRequest(const std::string& sessionToken, unsigned int cheatID, const std::string& releaseStream);

	uint32_t GetIdentifier() override;
	static void Register();
};