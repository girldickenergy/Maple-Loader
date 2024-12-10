#include "HandshakeResponse.h"

#include "entt.hpp"
#include "Fnv1a.h"

#include "../../Utilities/Reflection/TypeRegistrar.h"

static const TypeRegistrar<HandshakeResponse> registrar;

uint32_t HandshakeResponse::GetStaticIdentifier()
{
	return Hash32Fnv1aConst("HandshakeResponse");
}

const std::vector<unsigned char>& HandshakeResponse::GetKey()
{
	return m_Key;
}

const std::vector<unsigned char>& HandshakeResponse::GetEncryptedKey()
{
	return m_EncryptedKey;
}

const std::vector<unsigned char>& HandshakeResponse::GetEncryptedIV()
{
	return m_EncryptedIV;
}

uint32_t HandshakeResponse::GetIdentifier()
{
	return GetStaticIdentifier();
}

void HandshakeResponse::Register()
{
	entt::meta<HandshakeResponse>().type(GetStaticIdentifier())
		.data<&HandshakeResponse::m_Key>(Hash32Fnv1aConst("Key"))
		.data<&HandshakeResponse::m_EncryptedKey>(Hash32Fnv1aConst("EncryptedKey"))
		.data<&HandshakeResponse::m_EncryptedIV>(Hash32Fnv1aConst("EncryptedIV"));
}
