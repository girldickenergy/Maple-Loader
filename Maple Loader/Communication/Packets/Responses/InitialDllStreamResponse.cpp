#include "InitialDllStreamResponse.h"

#include <ThemidaSDK.h>

#include "../../../Utilities/Strings/StringUtilities.h"

InitialDllStreamResponse::InitialDllStreamResponse(const char* msg, size_t size, MatchedClient* matchedClient) : Response(msg, size)
{
	VM_SHARK_BLACK_START
	auto encrypted = StringUtilities::StringToByteArray(RawData[0]);

	encrypted.erase(encrypted.begin());

	std::string decrypted = matchedClient->aes->Decrypt(encrypted);

	std::vector<std::string> decryptedSplit = StringUtilities::Split(decrypted);
	Result = static_cast<InitialDllStreamResult>(decryptedSplit[0][0]);
	if (Result != InitialDllStreamResult::Success)
		return;
	
	decryptedSplit[1].erase(decryptedSplit[1].begin());
	decryptedSplit[2].erase(decryptedSplit[2].begin());

	AllocationSize = std::stoi(decryptedSplit[1]);

	std::vector<unsigned char> encryptedImports = StringUtilities::StringToByteArray(decryptedSplit[2]);
	
	std::vector<std::string> rawImports = StringUtilities::Split(matchedClient->aes->Decrypt(encryptedImports));
	for (int i = 0; i < rawImports.size(); i++)
	{
		if (i > 0)
			rawImports[i].erase(rawImports[i].begin());

		std::vector<std::string> importSplit = StringUtilities::Split(rawImports[i], " ");

		Imports.emplace_back(importSplit[0], importSplit[1], importSplit[2] == "1");
	}

	VM_SHARK_BLACK_END
}
