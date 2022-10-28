#include "LoginRequest.h"

#include "json.hpp"
#include "ThemidaSDK.h"

#include "../../Crypto/CryptoProvider.h"
#include "../../../Utilities/Strings/StringUtilities.h"
#include "../../../Utilities/Security/xorstr.hpp"
#include "../PacketType.h"

LoginRequest::LoginRequest(const std::string& username, const std::string& password, const std::string& loaderVersion, const std::string& hwid)
{
	this->username = username;
	this->password = password;
	this->loaderVersion = loaderVersion;
	this->hwid = hwid;
}

std::vector<unsigned char> LoginRequest::Serialize()
{
	VM_SHARK_BLACK_START
	STR_ENCRYPT_START

	nlohmann::json jsonPayload;

	jsonPayload[xorstr_("Username")] = username;
	jsonPayload[xorstr_("Password")] = password;
	jsonPayload[xorstr_("LoaderVersion")] = loaderVersion;
	jsonPayload[xorstr_("HWID")] = hwid;

	std::vector payload(CryptoProvider::GetInstance()->AESEncrypt(StringUtilities::StringToByteArray(jsonPayload.dump())));

	std::vector<unsigned char> packet;
	packet.push_back(static_cast<unsigned char>(PacketType::Login));
	packet.insert(packet.end(), payload.begin(), payload.end());

	STR_ENCRYPT_END
	VM_SHARK_BLACK_END

	return packet;
}
