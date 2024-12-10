#include "LoaderStreamRequest.h"

#include "entt.hpp"
#include "Fnv1a.h"

#include "../../Utilities/Reflection/TypeRegistrar.h"

static const TypeRegistrar<LoaderStreamRequest> registrar;

uint32_t LoaderStreamRequest::GetStaticIdentifier()
{
	return Hash32Fnv1aConst("LoaderStreamRequest");
}

LoaderStreamRequest::LoaderStreamRequest(const std::string& sessionToken, unsigned int cheatID, const std::string& releaseStream)
{
	m_SessionToken = sessionToken;
	m_CheatID = cheatID;
	m_ReleseStream = releaseStream;
}

uint32_t LoaderStreamRequest::GetIdentifier()
{
	return GetStaticIdentifier();
}

void LoaderStreamRequest::Register()
{
	if (registrar.IsRegistered)
		return;

	entt::meta<LoaderStreamRequest>().type(GetStaticIdentifier())
		.data<&LoaderStreamRequest::m_SessionToken>(Hash32Fnv1aConst("SessionToken"))
		.data<&LoaderStreamRequest::m_CheatID>(Hash32Fnv1aConst("CheatID"))
		.data<&LoaderStreamRequest::m_ReleseStream>(Hash32Fnv1aConst("ReleaseStream"));
}

