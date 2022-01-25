#include "GeneralHelper.h"

#include "../Communication/Communication.h"
#include "../UI/UI.h"

void GeneralHelper::ShutdownAndExit(bool disconnect)
{
	if (disconnect)
		Communication::TCPClient.finish();

	UI::Shutdown();

	ExitProcess(0);
}

std::string GeneralHelper::GetPCName()
{
    char buffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD lpnSize = MAX_COMPUTERNAME_LENGTH + 1;
    if (GetComputerNameA(buffer, &lpnSize) == FALSE)
        return "mplaudioservice";

    return std::string(buffer);
}
