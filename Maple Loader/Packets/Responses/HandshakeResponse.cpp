#include "HandshakeResponse.h"

#include "../../Globals.h"
#include "../../Utils/StringUtilities.h"

HandshakeResponse::HandshakeResponse(const char* msg, size_t size) : Response(msg, size)
{
	std::vector<unsigned char> encrypted = StringUtilities::StringToByteArray(RawData[1]);

	encrypted.erase(encrypted.begin());
	RawData[0].erase(RawData[0].begin());

	std::vector<unsigned char> decoded = Globals::RSA.Decode(encrypted, std::stoi(RawData[0]));

	IV = std::vector<unsigned char>(decoded.begin(), decoded.end() - 32);
	Key = std::vector<unsigned char>(decoded.begin() + 16, decoded.end());
}
