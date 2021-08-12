#include "FatalErrorResponse.h"

#include "../../Globals.h"
#include "../../Utils/StringUtilities.h"

FatalErrorResponse::FatalErrorResponse(const char* msg, size_t size) : Response(msg, size)
{
	VM_SHARK_BLACK_START
	std::vector<unsigned char> encrypted = StringUtilities::StringToByteArray(RawData[1]);
	
	encrypted.erase(encrypted.begin());
	RawData[0].erase(RawData[0].begin());
	
	ErrorMessage = StringUtilities::ByteArrayToString(Globals::RSA.Decode(encrypted, std::stoi(RawData[0])));
	VM_SHARK_BLACK_END
}
