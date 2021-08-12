#include "DllStreamResponse.h"

#include "../../Globals.h"
#include "../../ProcessHollowing/Write.h"
#include "../../Utils/StringUtilities.h"

DllStreamResponse::DllStreamResponse(const char* msg, size_t size, MatchedClient* matchedClient) : Response(msg, size)
{
	VM_SHARK_BLACK_START
	auto encrypted = StringUtilities::StringToByteArray(RawData[0]);

	encrypted.erase(encrypted.begin());

	std::string decrypted = matchedClient->aes->Decrypt(encrypted);

	std::vector<std::string> decryptedSplit = StringUtilities::Split(decrypted);
	Result = static_cast<DllStreamResult>(decryptedSplit[0][0]);
	if (Result != DllStreamResult::Success)
		return;
	
	decryptedSplit[1].erase(decryptedSplit[1].begin());

	std::vector<unsigned char> encryptedStream = StringUtilities::StringToByteArray(decryptedSplit[1]);
	
	ByteArray = StringUtilities::StringToByteArray(matchedClient->aes->Decrypt(encryptedStream));
	VM_SHARK_BLACK_END
}
