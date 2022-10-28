#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <stdio.h>

#include <WinSock2.h>
#include <curl.h>
#include <iostream>

#include <ThemidaSDK.h>

#include "Communication/Communication.h"
#include "UI/UI.h"
#include "Utilities/Security/xorstr.hpp"

#pragma optimize("", off)
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    VM_FISH_RED_START
    STR_ENCRYPT_START

	#ifdef _DEBUG
	    AllocConsole();
	    FILE* fp = nullptr;
	    freopen_s(&fp, xorstr_("CONIN$"), xorstr_("r"), stdin);
	    freopen_s(&fp, xorstr_("CONOUT$"), xorstr_("w"), stdout);
	    freopen_s(&fp, xorstr_("CONOUT$"), xorstr_("w"), stderr);
	#endif

    curl_global_init(CURL_GLOBAL_ALL);

    int protectionVar = 0x501938CA;
    CHECK_PROTECTION(protectionVar, 0x9CCC379)
    if (protectionVar != 0x9CCC379)
        return 0;

    if (!Communication::Connect())
    {
        MessageBoxA(NULL, xorstr_("Failed to connect to server!\nThe application will now exit."), xorstr_("Fatal error"), MB_ICONERROR | MB_OK);

        return 0;
    }

    if (!UI::Initialize())
    {
        MessageBoxA(NULL, xorstr_("Internal graphics error occurred!\nPlease make sure your graphics card drivers are up to date."), xorstr_("Fatal error"), MB_ICONERROR | MB_OK);

        return 0;
    }

    double lastCodeIntegrityCheckTime = 0.0;
    double lastDebuggerCheckTime = 0.0;
    while (!glfwWindowShouldClose(UI::GLFWWindow))
    {
        if (glfwGetTime() - lastCodeIntegrityCheckTime > 10.)
        {
            lastCodeIntegrityCheckTime = glfwGetTime();

            int codeIntegrityVar = 0x4878CC67;
            CHECK_CODE_INTEGRITY(codeIntegrityVar, 0x45C89C38)
            if (codeIntegrityVar != 0x45C89C38)
            {
                Communication::IntegritySignature1 -= 0x1;
                Communication::IntegritySignature2 -= 0x1;
                Communication::IntegritySignature3 -= 0x1;
            }
        }

        if (glfwGetTime() - lastDebuggerCheckTime > 1.)
        {
            lastDebuggerCheckTime = glfwGetTime();

            int debuggerVar = 0x5E95F73D;
            CHECK_DEBUGGER(debuggerVar, 0x723C5FC9)
            if (debuggerVar != 0x723C5FC9)
            {
                Communication::IntegritySignature1 -= 0x1;
                Communication::IntegritySignature2 -= 0x1;
                Communication::IntegritySignature3 -= 0x1;
            }
        }

        UI::Render();
    }

    UI::Shutdown();
    Communication::Disconnect();

    STR_ENCRYPT_END
    VM_FISH_RED_END

    return 0;
}
#pragma optimize("", on)
