#pragma once

#include <string>

#include "../IPacket.h"

class LoginRequest : public IPacket
{
	std::string m_Username;
	std::string m_Password;
	std::string m_LoaderVersion;
	std::string m_Hwid;

	static uint32_t GetStaticIdentifier();
public:
	LoginRequest(const std::string& username, const std::string& password, const std::string& loaderVersion, const std::string& hwid);

	uint32_t GetIdentifier() override;
	static void Register();
};