#include "LoaderStreamResponse.h"

#include "json.hpp"
#include "ThemidaSDK.h"

#include "../../../Utilities/Strings/StringUtilities.h"
#include "../../../Utilities/Security/xorstr.hpp"
#include "../../Crypto/CryptoProvider.h"

LoaderStreamResponse::LoaderStreamResponse(LoaderStreamResult result, const std::vector<unsigned char>& loaderData)
{
	this->result = result;
	this->loaderData = loaderData;
}

LoaderStreamResult LoaderStreamResponse::GetResult()
{
	return result;
}

const std::vector<unsigned char>& LoaderStreamResponse::GetLoaderData()
{
	return loaderData;
}

LoaderStreamResponse LoaderStreamResponse::Deserialize(const std::vector<unsigned char>& payload)
{
	VM_SHARK_BLACK_START
	STR_ENCRYPT_START

	nlohmann::json jsonPayload = nlohmann::json::parse(StringUtilities::ByteArrayToString(CryptoProvider::GetInstance()->AESDecrypt(payload)));

	LoaderStreamResult result = jsonPayload[xorstr_("Result")];
	LoaderStreamResponse response = LoaderStreamResponse(result, {});
	if (result == LoaderStreamResult::Success)
		response = LoaderStreamResponse(result, CryptoProvider::GetInstance()->Base64Decode(jsonPayload[xorstr_("LoaderData")]));

	STR_ENCRYPT_END
	VM_SHARK_BLACK_END

	return response;
}
