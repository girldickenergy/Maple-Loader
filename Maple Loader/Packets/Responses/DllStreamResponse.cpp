#include "DllStreamResponse.h"

#include "../../Utils/StringUtilities.h"

DllStreamResponse::DllStreamResponse(const char* msg, size_t size, MatchedClient* matchedClient) : Response(msg, size)
{
	auto encrypted = StringUtilities::StringToByteArray(RawData[0]);

	encrypted.erase(encrypted.begin());

	std::string decrypted = matchedClient->aes->Decrypt(encrypted);

	std::vector<unsigned char> byteArray = StringUtilities::StringToByteArray(decrypted);
}
