#include "LoginResponse.h"

#include "../../Utils/StringUtilities.h"

LoginResponse::LoginResponse(const char* msg, size_t size, MatchedClient* matchedClient) : Response(msg, size)
{
	auto encrypted = StringUtilities::StringToByteArray(RawData[0]);
	
	encrypted.erase(encrypted.begin());

	std::string decrypted = matchedClient->aes->Decrypt(encrypted);

	std::vector<std::string> decryptedSplit = StringUtilities::Split(decrypted);
	decryptedSplit[1].erase(decryptedSplit[1].begin());
	decryptedSplit[2].erase(decryptedSplit[2].begin());

	Result = static_cast<LoginResult>(decryptedSplit[0][0]);
	SessionToken = decryptedSplit[1];
	ExpiresAt = decryptedSplit[2];
}
