#pragma once

#include <vector>

#include "../IPacket.h"
#include "../RequestResult.h"

class LoaderStreamResponse : IPacket
{
	int32_t m_Result;
	std::vector<unsigned char> m_LoaderData;

	static uint32_t GetStaticIdentifier();
public:
	RequestResult GetResult();
	const std::vector<unsigned char>& GetLoaderData();

	uint32_t GetIdentifier() override;
	static void Register();
};
