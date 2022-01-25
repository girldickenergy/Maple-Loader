#include <WinSock2.h>
#include <curl.h>

#include "UI/UI.h"
#include "Communication/Communication.h"
#include "Utilities/GeneralHelper.h"

#include <ThemidaSDK.h>

#pragma optimize("", off)
int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prev_inst, LPSTR cmd_args, int show_cmd)
{
    VM_FISH_RED_START
    STR_ENCRYPT_START

    #ifdef _DEBUG
        AllocConsole();
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    #endif

    curl_global_init(CURL_GLOBAL_ALL);

    if (!Communication::ConnectToServer())
    {
        MessageBoxA(NULL, xor ("Failed to connect to server!\nThe application will now exit."), xor ("Discord"), MB_ICONERROR | MB_OK);

        return 0;
    }

    if (!UI::Initialize(inst))
    {
        MessageBoxA(NULL, xor ("Internal graphics error occurred!\nPlease make sure your graphics card drivers are up to date."), xor ("Discord"), MB_ICONERROR | MB_OK);

        return 0;
    }

    STR_ENCRYPT_END
    VM_FISH_RED_END

    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                done = true;
        }

        if (done)
            break;

        UI::Render();
    }

    GeneralHelper::ShutdownAndExit();

    return 0;
}
#pragma optimize("", on)
