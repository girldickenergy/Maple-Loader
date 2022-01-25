#include "UI.h"

#include <tchar.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include "../resource.h"
#include "StyleProvider.h"
#include "Widgets.h"
#include "../Utilities/Textures/TextureHelper.h"
#include "../Communication/Communication.h"
#include "../Utilities/GeneralHelper.h"
#include "../Communication/Packets/Requests/LoginRequest.h"
#include "../Communication/Packets/Requests/DllStreamRequest.h"
#include "../Utilities/Hardware/HardwareHelper.h"
#include "../Utilities/Crypto/CryptoHelper.h"

bool UI::createD3DDevice(HWND hWnd)
{
    if ((D3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;

    ZeroMemory(&D3DPresentParameters, sizeof(D3DPresentParameters));
    D3DPresentParameters.Windowed = TRUE;
    D3DPresentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    D3DPresentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
    D3DPresentParameters.EnableAutoDepthStencil = TRUE;
    D3DPresentParameters.AutoDepthStencilFormat = D3DFMT_D16;
    D3DPresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    if (D3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &D3DPresentParameters, &D3DDevice) < 0)
        return false;

    return true;
}

void UI::cleanupD3DDevice()
{
    if (D3DDevice)
    {
        D3DDevice->Release();
        D3DDevice = NULL;
    }

    if (D3D)
    {
        D3D->Release();
        D3D = NULL;
    }
}

void UI::resetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = D3DDevice->Reset(&D3DPresentParameters);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI UI::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
        case WM_SIZE:
            if (D3DDevice != NULL && wParam != SIZE_MINIMIZED)
            {
                D3DPresentParameters.BackBufferWidth = LOWORD(lParam);
                D3DPresentParameters.BackBufferHeight = HIWORD(lParam);

                resetDevice();
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU)
                return 0;
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
    }

    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

void UI::handleWindowDrag()
{
    if (ImGui::IsMouseClicked(0))
    {
        POINT point;
        RECT rect;

        GetCursorPos(&point);
        GetWindowRect(Window, &rect);

        dragOffsetX = point.x - rect.left;
        dragOffsetY = point.y - rect.top;

        canDrag = !ImGui::IsAnyItemHovered();
    }

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && canDrag)
    {
        POINT point;
        GetCursorPos(&point);

        SetWindowPos(Window, nullptr, point.x - dragOffsetX, point.y - dragOffsetY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}

void UI::updateWindowSize()
{
    bool loggedIn = Communication::CurrentState == States::LoggedIn || Communication::CurrentState == States::LoadingDLL;

    int width = loggedIn ? StyleProvider::MainWindowSize.x : StyleProvider::LoginWindowSize.x;
    int height = loggedIn ? StyleProvider::MainWindowSize.y : StyleProvider::LoginWindowSize.y;

    RECT pos;
    GetClientRect(GetDesktopWindow(), &pos);
    pos.left = (pos.right / 2) - (width / 2);
    pos.top = (pos.bottom / 2) - (height / 2);

    SetWindowRgn(Window, CreateRoundRectRgn(0, 0, width, height, 50, 50), true);

    ::SetWindowPos(Window, 0, pos.left, pos.top, width, height, SWP_NOOWNERZORDER | SWP_NOZORDER);
}

bool UI::Initialize(HINSTANCE hInst)
{
    ImGui_ImplWin32_EnableDpiAwareness();

    HICON hIcon = static_cast<HICON>(::LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR));

    windowClass = { sizeof(WNDCLASSEX), CS_CLASSDC, wndProc, 0L, 0L, GetModuleHandle(NULL), hIcon, NULL, NULL, NULL, _T("Discord"), NULL };
    ::RegisterClassEx(&windowClass);

    Window = ::CreateWindow(windowClass.lpszClassName, _T("Discord"), WS_POPUP, 0, 0, 0, 0, NULL, NULL, windowClass.hInstance, NULL);

    updateWindowSize();

    if (!createD3DDevice(Window))
    {
        cleanupD3DDevice();
        ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

        return false;
    }

    ::ShowWindow(Window, SW_SHOWDEFAULT);
    ::UpdateWindow(Window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplWin32_Init(Window);
    ImGui_ImplDX9_Init(D3DDevice);

    StyleProvider::Initialize();

    return true;
}

void UI::Render()
{
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    bool loggedIn = Communication::CurrentState == States::LoggedIn || Communication::CurrentState == States::LoadingDLL;
    bool wasLoggedIn = previousState == States::LoggedIn || previousState == States::LoadingDLL;

    previousState = Communication::CurrentState;

    handleWindowDrag();

    ImGuiStyle& style = ImGui::GetStyle();

    ImGui::SetNextWindowSize(loggedIn ? StyleProvider::MainWindowSize : StyleProvider::LoginWindowSize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin(xor ("Main Window"), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);
    {
        ImGui::PushFont(StyleProvider::FontDefault);

        const ImVec2 mainWindowSize = ImGui::GetCurrentWindow()->Size;

        //window bg
        ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0, 0), mainWindowSize, ImColor(loggedIn ? StyleProvider::MenuColour : StyleProvider::MenuColourDark));

        if (!loggedIn)
        {
            //render logo
            ImGui::PushFont(StyleProvider::FontHugeBold);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 10));
            ImGui::SetCursorPos(ImVec2(mainWindowSize.x / 2 - ImGui::CalcTextSize(xor ("Maple")).x / 2 - ImGui::GetFontSize() / 2, StyleProvider::WindowControlSize.y));
            ImGui::Image(StyleProvider::MapleLogoTexture, ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize()));
            ImGui::SameLine();
            ImGui::TextColored(StyleProvider::AccentColour, xor ("Maple"));
            ImGui::PopStyleVar();
            ImGui::PopFont();
            ImGui::PushFont(StyleProvider::FontSmall);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - style.ItemSpacing.y);
            ImGui::SetCursorPosX(mainWindowSize.x / 2 - ImGui::CalcTextSize(xor ("the quickest way to the top")).x / 2);
            ImGui::TextColored(StyleProvider::MottoColour, xor ("the quickest way to the top"));
            ImGui::PopFont();

            ImGui::Spacing();

            ImGui::SetCursorPosX(StyleProvider::LoginWindowPadding.x);
            ImGui::BeginChild(xor ("Login Child Window"), mainWindowSize - ImVec2(StyleProvider::LoginWindowPadding.x * 2, ImGui::GetCursorPosY() + StyleProvider::LoginWindowPadding.y), false, ImGuiWindowFlags_NoBackground);
            {
                const ImVec2 loginChildWindowSize = ImGui::GetCurrentWindow()->Size;
                const ImVec2 loginChildWindowPos = ImGui::GetCurrentWindow()->Pos;

                ImGui::GetCurrentWindow()->DrawList->AddRectFilled(loginChildWindowPos, loginChildWindowPos + loginChildWindowSize, ImColor(StyleProvider::MenuColourVeryDark), style.ChildRounding);

                ImGui::SetCursorPos(StyleProvider::LoginWindowInnerPadding);
                ImGui::BeginChild(xor ("Login Child Window Content"), loginChildWindowSize - StyleProvider::LoginWindowInnerPadding * 2, false, ImGuiWindowFlags_NoBackground);
                {
                    const ImVec2 loginChildWindowContentSize = ImGui::GetCurrentWindow()->Size;

                    bool loggingIn = Communication::CurrentState == States::LoggingIn;

                    if (loggingIn)
                        ImGui::BeginDisabled();

                    bool loginPressed = false;

                    ImGui::Text(xor ("Username"));
                    ImGui::PushItemWidth(loginChildWindowContentSize.x);
                    loginPressed |= ImGui::InputText(xor ("###username"), Communication::CurrentUser->Username, 24, ImGuiInputTextFlags_EnterReturnsTrue);

                    ImGui::Spacing();

                    ImGui::Text(xor ("Password"));
                    ImGui::PushItemWidth(loginChildWindowContentSize.x);
                    loginPressed |= ImGui::InputText(xor ("###password"), Communication::CurrentUser->Password, 256, ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue);

                    if (loggingIn)
                        ImGui::EndDisabled();

                    Widgets::Link(xor ("Forgot your password?"), xor ("https://maple.software/auth/forgotpassword"));

                    ImGui::Spacing();

                    if (loggingIn)
                        ImGui::BeginDisabled();

                    loginPressed |= Widgets::Button(xor (Communication::CurrentState == States::LoggingIn ? "Logging in..." : "Log in"), ImVec2(loginChildWindowContentSize.x, ImGui::GetFrameHeight()));

                    if (loggingIn)
                        ImGui::EndDisabled();

                    if (loginPressed)
                    {
                        std::string hwid = HardwareHelper::GetHWID();
                        if (hwid.empty())
                        {
                            MessageBoxA(Window, xor ("Failed to gather hardware information!\nPlease report this.\n\nThe application will now exit."), xor ("Discord"), MB_ICONERROR | MB_OK);

                            GeneralHelper::ShutdownAndExit();
                        }

                        LoginRequest loginPacket = LoginRequest(hwid, CryptoHelper::GetSHA256HashOfCurrentFile(), Communication::CurrentUser->Username, Communication::CurrentUser->Password, Communication::MatchedClient);

                        pipe_ret_t sendRet = Communication::TCPClient.sendBytes(loginPacket.Data);
                        if (!sendRet.success)
                        {
                            MessageBoxA(Window, xor ("Failed to communicate with the server!\nThe application will now exit."), xor ("Discord"), MB_ICONERROR | MB_OK);

                            GeneralHelper::ShutdownAndExit();
                        }

                        Communication::CurrentState = States::LoggingIn;
                    }

                    Widgets::LinkWithText(xor ("Sign up"), xor ("https://maple.software/auth/signup"), xor ("Need an account?"));
                }
                ImGui::EndChild();
            }
            ImGui::EndChild();
        }
        else
        {
            bool loadingDll = Communication::CurrentState == States::LoadingDLL;

            ImGui::BeginChild(xor ("Side Bar"), StyleProvider::MainWindowSideBarSize, false, ImGuiWindowFlags_NoBackground);
            {
                const ImVec2 sideBarSize = ImGui::GetCurrentWindow()->Size;
                const ImVec2 sideBarPos = ImGui::GetCurrentWindow()->Pos;

                ImGui::GetCurrentWindow()->DrawList->AddRectFilled(sideBarPos, sideBarPos + sideBarSize, ImColor(StyleProvider::MenuColourDark), style.ChildRounding, ImDrawFlags_RoundCornersBottomRight);

                ImGui::SetCursorPos(StyleProvider::Padding);
                ImGui::BeginChild(xor ("Side Bar Content"), sideBarSize - StyleProvider::Padding * 2, false, ImGuiWindowFlags_NoBackground);
                {
                    const ImVec2 sideBarContentSize = ImGui::GetCurrentWindow()->Size;

                    //render logo
                    ImGui::PushFont(StyleProvider::FontHugeBold);
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 10));
                    ImGui::SetCursorPosX(sideBarContentSize.x / 2 - ImGui::CalcTextSize(xor ("Maple")).x / 2 - ImGui::GetFontSize() / 2);
                    ImGui::Image(StyleProvider::MapleLogoTexture, ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize()));
                    ImGui::SameLine();
                    ImGui::TextColored(StyleProvider::AccentColour, xor ("Maple"));
                    ImGui::PopStyleVar();
                    ImGui::PopFont();
                    ImGui::PushFont(StyleProvider::FontSmall);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - style.ItemSpacing.y);
                    ImGui::SetCursorPosX(sideBarContentSize.x / 2 - ImGui::CalcTextSize(xor ("the quickest way to the top")).x / 2);
                    ImGui::TextColored(StyleProvider::MottoColour, xor ("the quickest way to the top"));
                    ImGui::PopFont();

                    ImGui::Spacing();

                    ImGui::BeginChild(xor ("Side Bar User Info"), ImVec2(sideBarContentSize.x, 100), false, ImGuiWindowFlags_NoBackground);
                    {
                        const ImVec2 sideBarUserInfoSize = ImGui::GetCurrentWindow()->Size;
                        const ImVec2 sideBarUserInfoPos = ImGui::GetCurrentWindow()->Pos;

                        ImGui::GetCurrentWindow()->DrawList->AddRectFilled(sideBarUserInfoPos, sideBarUserInfoPos + sideBarUserInfoSize, ImColor(StyleProvider::MenuColourVeryDark), style.ChildRounding);

                        ImGui::GetWindowDrawList()->AddImageRounded(StyleProvider::AvatarTexture, sideBarUserInfoPos + ImVec2(sideBarUserInfoSize.y / 4, sideBarUserInfoSize.y / 4), sideBarUserInfoPos + ImVec2(sideBarUserInfoSize.y / 4 + sideBarUserInfoSize.y / 2, sideBarUserInfoSize.y / 4 + sideBarUserInfoSize.y / 2), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), style.FrameRounding);

                        ImGui::PushFont(StyleProvider::FontDefaultBold);
                        ImGui::SetCursorPos(ImVec2(sideBarUserInfoSize.y / 4 + sideBarUserInfoSize.y / 2 + style.ItemSpacing.x, sideBarUserInfoSize.y / 2 - style.ItemSpacing.y / 4 - ImGui::CalcTextSize("Welcome back").y));
                        ImGui::Text(xor ("Welcome back"));

                        ImGui::SetCursorPos(ImVec2(sideBarUserInfoSize.y / 4 + sideBarUserInfoSize.y / 2 + style.ItemSpacing.x, sideBarUserInfoSize.y / 2 + style.ItemSpacing.y / 4));
                        Widgets::Link(Communication::CurrentUser->Username, "https://maple.software/dashboard", false);
                        ImGui::PopFont();
                    }
                    ImGui::EndChild();

                    ImGui::BeginChild(xor ("Side Bar Cheats"), ImVec2(sideBarContentSize.x, sideBarContentSize.y - ImGui::GetCursorPosY()), false, ImGuiWindowFlags_NoBackground);
                    {
                        const ImVec2 sideBarCheatsSize = ImGui::GetCurrentWindow()->Size;
                        const ImVec2 sideBarCheatsPos = ImGui::GetCurrentWindow()->Pos;

                        ImGui::GetCurrentWindow()->DrawList->AddRectFilled(sideBarCheatsPos, sideBarCheatsPos + sideBarCheatsSize, ImColor(StyleProvider::MenuColourVeryDark), style.ChildRounding);

                        ImGui::SetCursorPos(StyleProvider::Padding);
                        ImGui::BeginChild(xor ("Side Bar Cheats Content"), sideBarCheatsSize - StyleProvider::Padding * 2, false, ImGuiWindowFlags_NoBackground);
                        {
                            if (shouldSelectFirstGame)
                            {
                                shouldSelectFirstGame = false;
                                ImGui::SetNextTreeNodeOpen(true);
                            }

                            for (int i = 0; i < Communication::CurrentUser->Games.size(); i++)
                            {
                                ImGui::PushFont(StyleProvider::FontDefaultBold);

                                if (loadingDll)
                                    ImGui::BeginDisabled();

                                bool nodeOpened = Widgets::TreeNode(Communication::CurrentUser->Games[i]->Name.c_str(), Communication::CurrentUser->Games[i]->Icon);

                                if (loadingDll)
                                    ImGui::EndDisabled();

                                ImGui::PopFont();

                                if (nodeOpened)
                                {
                                    for (int j = 0; j < Communication::CurrentUser->Cheats.size(); j++)
                                    {
                                        if (Communication::CurrentUser->Cheats[j]->GameID != Communication::CurrentUser->Games[i]->ID)
                                            continue;

                                        ImGui::PushFont(StyleProvider::FontDefaultSemiBold);

                                        if (loadingDll)
                                            ImGui::BeginDisabled();

                                        bool selected = Widgets::Selectable(Communication::CurrentUser->Cheats[j]->Name.c_str(), Communication::CurrentUser->CurrentCheat->ID == Communication::CurrentUser->Cheats[j]->ID, ImGuiSelectableFlags_SpanAllColumns);

                                        if (loadingDll)
                                            ImGui::EndDisabled();

                                        ImGui::PopFont();

                                        if (selected)
                                        {
                                            Communication::CurrentUser->CurrentCheat = Communication::CurrentUser->Cheats[j];

                                            for (Game* game : Communication::CurrentUser->Games)
                                                if (game->ID == Communication::CurrentUser->CurrentCheat->GameID)
                                                    Communication::CurrentUser->CurrentGame = game;
                                        }
                                    }

                                    ImGui::TreePop();
                                }
                            }
                        }
                        ImGui::EndChild();
                    }
                    ImGui::EndChild();
                }
                ImGui::EndChild();
            }
            ImGui::EndChild();

            ImGui::SetCursorPos(ImVec2(StyleProvider::MainWindowSideBarSize.x, 0));
            ImGui::BeginChild(xor ("Cheat Banner"), StyleProvider::CheatBannerSize, false, ImGuiWindowFlags_NoBackground);
            {
                const ImVec2 cheatBannerSize = ImGui::GetCurrentWindow()->Size;

                ImGui::Image(Communication::CurrentUser->CurrentGame->Banner, cheatBannerSize);
                Widgets::Gradient(ImVec2(0, cheatBannerSize.y), ImVec2(cheatBannerSize.x, 0), StyleProvider::CheatBannerGradientStartColour, StyleProvider::CheatBannerGradientEndColour);

                ImGui::SetCursorPos(StyleProvider::Padding);
                ImGui::BeginChild(xor ("Cheat Banner Content"), cheatBannerSize - StyleProvider::Padding * 2, false, ImGuiWindowFlags_NoBackground);
                {
                    const ImVec2 cheatBannerContentSize = ImGui::GetCurrentWindow()->Size;

                    ImGui::PushFont(StyleProvider::FontHugeBold);
                    ImGui::Text(Communication::CurrentUser->CurrentCheat->Name.c_str());
                    ImGui::PopFont();

                    ImGui::SetCursorPosY(ImGui::GetCursorPos().y - ImGui::GetStyle().ItemSpacing.y);
                    ImGui::PushFont(StyleProvider::FontDefaultSemiBold);
                    ImGui::Text(xor ("for %s"), Communication::CurrentUser->CurrentGame->Name.c_str());

                    const float priceHeight = ImGui::CalcTextSize(xor ("10 euro per month")).y;
                    ImGui::SetCursorPosY(cheatBannerContentSize.y - ImGui::GetFrameHeight() / 2 - priceHeight / 2);
                    ImGui::Text(xor("%i euro per month"), Communication::CurrentUser->CurrentCheat->Price);
                    ImGui::PopFont();

                    const float widgetWidth = cheatBannerContentSize.x * 0.25f;

                    if (loadingDll)
                        ImGui::BeginDisabled();

                    ImGui::SetCursorPos(cheatBannerContentSize - ImVec2(widgetWidth, ImGui::GetFrameHeight() * 2 + style.ItemSpacing.y));
                    ImGui::PushItemWidth(widgetWidth);
                    Widgets::Combo(xor ("###ReleaseStream"), &Communication::CurrentUser->CurrentCheat->CurrentStream, Communication::CurrentUser->CurrentCheat->ReleaseStreams);

                    if (loadingDll)
                        ImGui::EndDisabled();

                    ImGui::SetCursorPos(cheatBannerContentSize - ImVec2(widgetWidth, ImGui::GetFrameHeight()));
                    if (strcmp(Communication::CurrentUser->CurrentCheat->ExpiresAt.c_str(), xor ("not subscribed")) == 0)
                    {
                        if (ImGui::Button(xor ("Buy now"), ImVec2(widgetWidth, ImGui::GetFrameHeight())))
                            ShellExecuteA(NULL, xor ("open"), xor ("https://maple.software/dashboard/store"), NULL, NULL, SW_SHOWNORMAL);
                    }
                    else
                    {
                        if (loadingDll)
                            ImGui::BeginDisabled();

                        bool loadClicked = Widgets::Button(xor(Communication::CurrentState == States::LoadingDLL ? "Loading..." : "Load"), ImVec2(widgetWidth, ImGui::GetFrameHeight()));

                        if (loadingDll)
                            ImGui::EndDisabled();

                        if (loadClicked)
                        {
                            DllStreamRequest dllStream = DllStreamRequest(Communication::CurrentUser->CurrentCheat->ID, Communication::CurrentUser->CurrentCheat->ReleaseStreams[Communication::CurrentUser->CurrentCheat->CurrentStream], Communication::MatchedClient);

                            pipe_ret_t sendRet = Communication::TCPClient.sendBytes(dllStream.Data);
                            if (!sendRet.success)
                            {
                                MessageBoxA(Window, xor ("Failed to communicate with the server!\nThe application will now exit."), xor ("Discord"), MB_ICONERROR | MB_OK);

                                GeneralHelper::ShutdownAndExit();
                            }

                            Communication::CurrentState = States::LoadingDLL;
                        }
                    }
                }
                ImGui::EndChild();
            }
            ImGui::EndChild();

            ImGui::SetCursorPos(ImVec2(StyleProvider::MainWindowSideBarSize.x, StyleProvider::CheatBannerSize.y));
            ImGui::BeginChild(xor ("Cheat Info"), ImVec2(mainWindowSize.x - StyleProvider::MainWindowSideBarSize.x, mainWindowSize.y - StyleProvider::CheatBannerSize.y), false, ImGuiWindowFlags_NoBackground);
            {
                const ImVec2 cheatInfoSize = ImGui::GetCurrentWindow()->Size;

                ImGui::SetCursorPos(StyleProvider::Padding);
                ImGui::BeginChild(xor ("Cheat Info Content"), cheatInfoSize - StyleProvider::Padding * 2, false, ImGuiWindowFlags_NoBackground);
                {
                    ImGui::PushFont(StyleProvider::FontBigBold);
                    ImGui::Text(xor ("Subscription expires at"));
                    ImGui::PopFont();

                    ImGui::PushFont(StyleProvider::FontDefaultSemiBold);
                    ImGui::Text(Communication::CurrentUser->CurrentCheat->ExpiresAt.c_str());
                    if (strcmp(Communication::CurrentUser->CurrentCheat->ExpiresAt.c_str(), xor ("not subscribed")) == 0)
                    {
                        ImGui::SameLine();
                        Widgets::Link(xor ("subscribe now"), xor ("https://maple.software/dashboard/store"), false);
                    }
                    ImGui::PopFont();

                    ImGui::Spacing();

                    ImGui::PushFont(StyleProvider::FontBigBold);
                    ImGui::Text(xor ("Status"));
                    ImGui::PopFont();

                    ImGui::PushFont(StyleProvider::FontDefaultSemiBold);
                    switch (Communication::CurrentUser->CurrentCheat->Status)
                    {
                        case CheatStatus::UpToDate:
                            ImGui::TextColored(ImVec4(0, 1, 0, 1), xor ("up to date"));
                            break;
                        case CheatStatus::Outdated:
                            ImGui::TextColored(ImVec4(ImColor(255, 69, 0, 255)), xor ("outdated"));
                            break;
                        case CheatStatus::Detected:
                            ImGui::TextColored(ImVec4(1, 0, 0, 1), xor ("detected"));
                            break;
                    }
                    ImGui::PopFont();

                    ImGui::Spacing();

                    ImGui::PushFont(StyleProvider::FontBigBold);
                    ImGui::Text(xor ("Features"));
                    ImGui::PopFont();

                    ImGui::PushFont(StyleProvider::FontDefaultSemiBold);
                    ImGui::TextWrapped(Communication::CurrentUser->CurrentCheat->Features.c_str());
                    Widgets::Link(xor ("Visit our website for more information"), xor ("https://maple.software/"), false);
                    ImGui::PopFont();
                }
                ImGui::EndChild();
            }
            ImGui::EndChild();
        }

        ImGui::SetCursorPos(ImVec2(mainWindowSize.x - StyleProvider::WindowControlSize.x * 2, 0));
        ImGui::BeginChild(xor ("Window Controls"), ImVec2(StyleProvider::WindowControlSize.x * 2, StyleProvider::WindowControlSize.y), false, ImGuiWindowFlags_NoBackground);
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            ImGui::PushStyleColor(ImGuiCol_Text, StyleProvider::WindowControlTextColour);

            ImGui::PushStyleColor(ImGuiCol_Button, StyleProvider::MinimizeButtonColour);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, StyleProvider::MinimizeButtonHoveredColour);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, StyleProvider::MinimizeButtonActiveColour);

            if (Widgets::Button(xor ("_"), StyleProvider::WindowControlSize))
                ShowWindow(Window, SW_MINIMIZE);

            ImGui::PopStyleColor(3);

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, StyleProvider::CloseButtonColour);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, StyleProvider::CloseButtonHoveredColour);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, StyleProvider::CloseButtonActiveColour);

            if (Widgets::Button(xor ("x"), StyleProvider::WindowControlSize))
                GeneralHelper::ShutdownAndExit();

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);
        }
        ImGui::EndChild();

        ImGui::PopFont();
    }
    ImGui::End();

    ImGui::EndFrame();

    D3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    D3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

    D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(65, 65, 65, 255);
    D3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
    if (D3DDevice->BeginScene() >= 0)
    {
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        D3DDevice->EndScene();
    }

    if (loggedIn != wasLoggedIn)
        updateWindowSize();

    if (D3DDevice->Present(NULL, NULL, NULL, NULL) == D3DERR_DEVICELOST && D3DDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
        resetDevice();
}

void UI::Shutdown()
{
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    cleanupD3DDevice();
    ::DestroyWindow(Window);
    ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}
