#include "HandshakeRequest.h"

#include "entt.hpp"
#include "Fnv1a.h"

#include "../PacketRegistrar.h"

static const PacketRegistrar<HandshakeRequest> registrar;

uint32_t HandshakeRequest::GetStaticIdentifier()
{
	return Hash32Fnv1aConst("LoginRequest");
}

HandshakeRequest::HandshakeRequest(const std::vector<uint32_t>& randomJunk)
{
	m_RandomJunk = randomJunk;
}

uint32_t HandshakeRequest::GetIdentifier()
{
	return GetStaticIdentifier();
}

void HandshakeRequest::Register()
{
	if (registrar.IsRegistered)
		return;

	entt::meta<HandshakeRequest>().type(GetStaticIdentifier())
		.data<&HandshakeRequest::m_RandomJunk>(Hash32Fnv1aConst("RandomJunk"));
}
