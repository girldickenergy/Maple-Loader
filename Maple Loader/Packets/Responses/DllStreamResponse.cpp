#include "DllStreamResponse.h"

#include "../../Globals.h"
#include "../../ProcessHollowing/ProcessHollowing.h"
#include "../../ProcessHollowing/Write.h"
#include "../../Utils/StringUtilities.h"
#include "../../ProcessHollowing/Data.h"
#include "../../UI/UI.h"

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
	HANDLE hProcess = ProcessHollowing::CreateHollowedProcess(InjectorData::Injector_protected_exe);
	if (hProcess == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(UI::Window, xor ("Process handle is invalid"), xor ("Maple Loader"), MB_ICONERROR | MB_OK);
		Globals::LoaderState = LoaderStates::LoggedIn;
		return;
	}
	
	// Amazing, now we wait for the PE to load fully because Themida can take a while...
	Sleep(1500);
	
	// Now we read the memory of the ghost process, write the binary to it, and the player data.
	Write::WriteData(hProcess, &byteArray, matchedClient);
}
