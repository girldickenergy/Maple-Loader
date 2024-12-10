#include "LoaderStreamResponse.h"

#include "entt.hpp"
#include "Fnv1a.h"

#include "../../Utilities/Reflection/TypeRegistrar.h"

static const TypeRegistrar<LoaderStreamResponse> registrar;

uint32_t LoaderStreamResponse::GetStaticIdentifier()
{
	return Hash32Fnv1aConst("LoaderStreamResponse");
}

RequestResult LoaderStreamResponse::GetResult()
{
	return static_cast<RequestResult>(m_Result);
}

const std::vector<unsigned char>& LoaderStreamResponse::GetLoaderData()
{
	return m_LoaderData;
}

uint32_t LoaderStreamResponse::GetIdentifier()
{
	return GetStaticIdentifier();
}

void LoaderStreamResponse::Register()
{
	if (registrar.IsRegistered)
		return;

	entt::meta<LoaderStreamResponse>().type(GetStaticIdentifier())
		.data<&LoaderStreamResponse::m_Result>(Hash32Fnv1aConst("Result"))
		.data<&LoaderStreamResponse::m_LoaderData>(Hash32Fnv1aConst("LoaderData"));
}