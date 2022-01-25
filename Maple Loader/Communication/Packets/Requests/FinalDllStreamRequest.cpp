#include "FinalDllStreamRequest.h"

FinalDllStreamRequest::FinalDllStreamRequest(unsigned int allocationBase, std::vector<unsigned int> imports, MatchedClient* matchedClient) : Request(RequestType::FinalDllStream)
{
	VM_SHARK_BLACK_START
	std::vector<unsigned char> data;

	AddString(std::to_string(allocationBase), &data);

	for (int i = 0; i < imports.size(); i++)
		AddString(std::to_string(imports[i]), &data);

	AddString(matchedClient->aes->Encrypt(data));
	VM_SHARK_BLACK_END
}
