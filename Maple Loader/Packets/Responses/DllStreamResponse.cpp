#include "DllStreamResponse.h"

#include "../../Utils/StringUtilities.h"

DllStreamResponse::DllStreamResponse(const char* msg, size_t size, MatchedClient* matchedClient) : Response(msg, size)
{
	auto encrypted = StringUtilities::StringToByteArray(RawData[0]);

	encrypted.erase(encrypted.begin());

	std::string decrypted = matchedClient->aes->Decrypt(encrypted);

	std::vector<std::string> decryptedSplit = StringUtilities::Split(decrypted);
	Result = static_cast<DllStreamResult>(decryptedSplit[0][0]);
	if (Result != DllStreamResult::Success)
		return;
	
	decryptedSplit[1].erase(decryptedSplit[1].begin());

	std::vector<unsigned char> byteArray = StringUtilities::StringToByteArray(decryptedSplit[1]);
}
