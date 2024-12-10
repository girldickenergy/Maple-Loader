#include "LoginRequest.h"

#include "entt.hpp"
#include "Fnv1a.h"

#include "../PacketRegistrar.h"

static const PacketRegistrar<LoginRequest> registrar;

uint32_t LoginRequest::GetStaticIdentifier()
{
	return Hash32Fnv1aConst("LoginRequest");
}

LoginRequest::LoginRequest(const std::string& username, const std::string& password, const std::string& loaderVersion, const std::string& hwid)
{
	m_Username = username;
	m_Password = password;
	m_LoaderVersion = loaderVersion;
	m_Hwid = hwid;
}

uint32_t LoginRequest::GetIdentifier()
{
	return GetStaticIdentifier();
}

void LoginRequest::Register()
{
	if (registrar.IsRegistered)
		return;

	entt::meta<LoginRequest>().type(GetStaticIdentifier())
		.data<&LoginRequest::m_Username>(Hash32Fnv1aConst("Username"))
		.data<&LoginRequest::m_Password>(Hash32Fnv1aConst("Password"))
		.data<&LoginRequest::m_Hwid>(Hash32Fnv1aConst("HWID"))
		.data<&LoginRequest::m_LoaderVersion>(Hash32Fnv1aConst("LoaderVersion"));
}
