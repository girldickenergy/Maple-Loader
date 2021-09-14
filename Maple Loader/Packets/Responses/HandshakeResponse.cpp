#include "HandshakeResponse.h"

#include "../../Globals.h"
#include "../../Utils/StringUtilities.h"
#include <chrono>

HandshakeResponse::HandshakeResponse(const char* msg, size_t size) : Response(msg, size)
{
	VM_SHARK_BLACK_START
	std::chrono::milliseconds msEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

	std::vector<unsigned char> encrypted = StringUtilities::StringToByteArray(RawData[1]);

	encrypted.erase(encrypted.begin());
	RawData[0].erase(RawData[0].begin());

	std::vector<unsigned char> decoded = Globals::RSA.Decode(encrypted, std::stoi(RawData[0]));

	IV = std::vector<unsigned char>(decoded.begin(), decoded.end() - (32 + (decoded.size() - 32 - 16)));
	Key = std::vector<unsigned char>(decoded.begin() + 16, decoded.end() - (decoded.size() - 32 - 16));
	std::string epoch = std::string(decoded.begin() + 48, decoded.end());
	long long realEpoch = std::stoll(epoch);
	//realEpoch ^= -5909373644027609361;
	//realEpoch += 234515;

	if (decoded.size() < 48 || decoded.size() > 100 || IV.size() != 16 || Key.size() != 32)
	{
		Result = HandshakeResult::InternalError;
		return;
	}

	if (std::abs(msEpoch.count() - (realEpoch*2)^0xDA) > 5000)
	{
		Result = HandshakeResult::EpochTimedOut;
		return;
	}

	Result = HandshakeResult::Success;
	VM_SHARK_BLACK_END
}
