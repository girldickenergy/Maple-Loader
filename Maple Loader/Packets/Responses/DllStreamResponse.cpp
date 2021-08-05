#include "DllStreamResponse.h"

#include "../../Utils/StringUtilities.h"
#include "../../RunPE/RunPE.cpp"
#include "../../RunPE/Write.cpp"
#include "../../RunPE/Data.h"

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

	// Dll stream has been fully decrypted and received. Now we RunPE the injector and WPM the binary into it!
	RunPE::RunPE::RunPortableExecutable(RunPE::Injector_protected_exe);

	// Amazing, now we wait for the PE to load fully because Themida can take a while...
	Sleep(1500);

	// Now we read the memory of the ghost process, write the binary to it, and the player data.
	RunPE::Write::WriteData(&byteArray, matchedClient);
}
