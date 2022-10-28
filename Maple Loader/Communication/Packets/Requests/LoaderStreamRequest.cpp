#include "LoaderStreamRequest.h"

#include "json.hpp"
#include "ThemidaSDK.h"

#include "../../Crypto/CryptoProvider.h"
#include "../../../Utilities/Strings/StringUtilities.h"
#include "../../../Utilities/Security/xorstr.hpp"
#include "../PacketType.h"

LoaderStreamRequest::LoaderStreamRequest(const std::string& sessionToken, unsigned int cheatID)
{
	this->sessionToken = sessionToken;
	this->cheatID = cheatID;
}

std::vector<unsigned char> LoaderStreamRequest::Serialize()
{
	VM_SHARK_BLACK_START
	STR_ENCRYPT_START

	nlohmann::json jsonPayload;

	jsonPayload[xorstr_("SessionToken")] = sessionToken;
	jsonPayload[xorstr_("CheatID")] = cheatID;

	std::vector payload(CryptoProvider::GetInstance()->AESEncrypt(StringUtilities::StringToByteArray(jsonPayload.dump())));

	std::vector<unsigned char> packet;
	packet.push_back(static_cast<unsigned char>(PacketType::LoaderStream));
	packet.insert(packet.end(), payload.begin(), payload.end());

	STR_ENCRYPT_END
	VM_SHARK_BLACK_END

	return packet;
}
