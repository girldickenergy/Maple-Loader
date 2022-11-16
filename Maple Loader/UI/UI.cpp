#include "UI.h"

#include <shellapi.h>
#include <filesystem>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw3native.h>

#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "StyleProvider.h"
#include "Widgets/Widgets.h"
#include "../Utilities/Security/xorstr.hpp"
#include "../Communication/Communication.h"

void UI::setWindowSize(int width, int height)
{
    RECT pos;
    GetClientRect(GetDesktopWindow(), &pos);
    pos.left = (pos.right / 2) - (width / 2);
    pos.top = (pos.bottom / 2) - (height / 2);

    SetWindowRgn(NativeWindow, CreateRoundRectRgn(0, 0, width, height, 50, 50), true);

    ::SetWindowPos(NativeWindow, 0, pos.left, pos.top, width, height, SWP_NOOWNERZORDER | SWP_NOZORDER);
}

void UI::handleWindowDrag()
{
    if (ImGui::IsMouseClicked(0))
    {
        POINT point;
        RECT rect;

        GetCursorPos(&point);
        GetWindowRect(NativeWindow, &rect);

        dragOffsetX = point.x - rect.left;
        dragOffsetY = point.y - rect.top;

        canDrag = !ImGui::IsAnyItemHovered();
    }

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && canDrag)
    {
        POINT point;
        GetCursorPos(&point);

        SetWindowPos(NativeWindow, nullptr, point.x - dragOffsetX, point.y - dragOffsetY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}

bool UI::Initialize()
{
    ImGui_ImplWin32_EnableDpiAwareness();

    if (!glfwInit())
        return false;

    const char* glsl_version = xorstr_("#version 130");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    char filepath[MAX_PATH];
    GetModuleFileNameA(NULL, filepath, MAX_PATH);
    const std::string filename = std::filesystem::path(filepath).stem().string();
    GLFWWindow = glfwCreateWindow(600, 400, filename.c_str(), NULL, NULL);
    if (!GLFWWindow)
        return false;
	
    NativeWindow = glfwGetWin32Window(GLFWWindow);
    if (!NativeWindow)
        return false;

    setWindowSize(static_cast<int>(StyleProvider::LoginWindowSize.x), static_cast<int>(StyleProvider::LoginWindowSize.y));

    glfwMakeContextCurrent(GLFWWindow);
    glfwSwapInterval(1); //vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
	
    ImGui_ImplGlfw_InitForOpenGL(GLFWWindow, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    StyleProvider::Initialize();

	return true;
}

void UI::Render()
{
    glfwPollEvents();
    handleWindowDrag();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const bool loggedIn = Communication::GetState() == States::LoggedIn || Communication::GetState() == States::LoadingPayload;

    const ImGuiStyle& style = ImGui::GetStyle();

    ImGui::SetNextWindowSize(loggedIn ? StyleProvider::MainWindowSize : StyleProvider::LoginWindowSize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin(xorstr_("Main Window"), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);
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
            ImGui::SetCursorPos(ImVec2(mainWindowSize.x / 2 - ImGui::CalcTextSize(xorstr_("Maple")).x / 2 - ImGui::GetFontSize() / 2, StyleProvider::WindowControlSize.y));
            ImGui::Image(StyleProvider::MapleLogoTexture, ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize()));
            ImGui::SameLine();
            ImGui::TextColored(StyleProvider::AccentColour, xorstr_("Maple"));
            ImGui::PopStyleVar();
            ImGui::PopFont();
            ImGui::PushFont(StyleProvider::FontSmall);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - style.ItemSpacing.y);
            ImGui::SetCursorPosX(mainWindowSize.x / 2 - ImGui::CalcTextSize(xorstr_("the quickest way to the top")).x / 2);
            ImGui::TextColored(StyleProvider::MottoColour, xorstr_("the quickest way to the top"));
            ImGui::PopFont();

            ImGui::Spacing();

            ImGui::SetCursorPosX(StyleProvider::LoginWindowPadding.x);
            ImGui::BeginChild(xorstr_("Login Child Window"), mainWindowSize - ImVec2(StyleProvider::LoginWindowPadding.x * 2, ImGui::GetCursorPosY() + StyleProvider::LoginWindowPadding.y), false, ImGuiWindowFlags_NoBackground);
            {
                const ImVec2 loginChildWindowSize = ImGui::GetCurrentWindow()->Size;
                const ImVec2 loginChildWindowPos = ImGui::GetCurrentWindow()->Pos;

                ImGui::GetCurrentWindow()->DrawList->AddRectFilled(loginChildWindowPos, loginChildWindowPos + loginChildWindowSize, ImColor(StyleProvider::MenuColourVeryDark), style.ChildRounding);

                ImGui::SetCursorPos(StyleProvider::LoginWindowInnerPadding);
                ImGui::BeginChild(xorstr_("Login Child Window Content"), loginChildWindowSize - StyleProvider::LoginWindowInnerPadding * 2, false, ImGuiWindowFlags_NoBackground);
                {
                    const ImVec2 loginChildWindowContentSize = ImGui::GetCurrentWindow()->Size;

                    const bool loggingIn = Communication::GetState() == States::LoggingIn;

                    if (loggingIn)
                        ImGui::BeginDisabled();

                    bool loginPressed = false;

                    ImGui::Text(xorstr_("Username"));
                    ImGui::PushItemWidth(loginChildWindowContentSize.x);
                    loginPressed |= ImGui::InputText(xorstr_("###username"), Communication::LoginUsername, 24, ImGuiInputTextFlags_EnterReturnsTrue);

                    ImGui::Spacing();

                    ImGui::Text(xorstr_("Password"));
                    ImGui::PushItemWidth(loginChildWindowContentSize.x);
                    loginPressed |= ImGui::InputText(xorstr_("###password"), Communication::LoginPassword, 256, ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue);

                    if (loggingIn)
                        ImGui::EndDisabled();

                    Widgets::Link(xorstr_("Forgot your password?"), xorstr_("https://maple.software/auth/passwordRecovery"));

                    ImGui::Spacing();

                    if (loggingIn)
                        ImGui::BeginDisabled();

                    loginPressed |= Widgets::Button(Communication::GetState() == States::LoggingIn ? xorstr_("Logging in...") : xorstr_("Log in"), ImVec2(loginChildWindowContentSize.x, ImGui::GetFrameHeight()));

                    if (loggingIn)
                        ImGui::EndDisabled();

                    if (loginPressed)
                        Communication::LogIn();

                    Widgets::LinkWithText(xorstr_("Sign up"), xorstr_("https://maple.software/auth/signup"), xorstr_("Need an account?"));
                }
                ImGui::EndChild();
            }
            ImGui::EndChild();
        }
        else
        {
	        const bool loadingImage = Communication::GetState() == States::LoadingPayload;

            ImGui::BeginChild(xorstr_("Side Bar"), StyleProvider::MainWindowSideBarSize, false, ImGuiWindowFlags_NoBackground);
            {
                const ImVec2 sideBarSize = ImGui::GetCurrentWindow()->Size;
                const ImVec2 sideBarPos = ImGui::GetCurrentWindow()->Pos;

                ImGui::GetCurrentWindow()->DrawList->AddRectFilled(sideBarPos, sideBarPos + sideBarSize, ImColor(StyleProvider::MenuColourDark), style.ChildRounding, ImDrawFlags_RoundCornersBottomRight);

                ImGui::SetCursorPos(StyleProvider::Padding);
                ImGui::BeginChild(xorstr_("Side Bar Content"), sideBarSize - StyleProvider::Padding * 2, false, ImGuiWindowFlags_NoBackground);
                {
                    const ImVec2 sideBarContentSize = ImGui::GetCurrentWindow()->Size;

                    //render logo
                    ImGui::PushFont(StyleProvider::FontHugeBold);
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 10));
                    ImGui::SetCursorPosX(sideBarContentSize.x / 2 - ImGui::CalcTextSize(xorstr_("Maple")).x / 2 - ImGui::GetFontSize() / 2);
                    ImGui::Image(StyleProvider::MapleLogoTexture, ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize()));
                    ImGui::SameLine();
                    ImGui::TextColored(StyleProvider::AccentColour, xorstr_("Maple"));
                    ImGui::PopStyleVar();
                    ImGui::PopFont();
                    ImGui::PushFont(StyleProvider::FontSmall);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - style.ItemSpacing.y);
                    ImGui::SetCursorPosX(sideBarContentSize.x / 2 - ImGui::CalcTextSize(xorstr_("the quickest way to the top")).x / 2);
                    ImGui::TextColored(StyleProvider::MottoColour, xorstr_("the quickest way to the top"));
                    ImGui::PopFont();

                    ImGui::Spacing();

                    ImGui::BeginChild(xorstr_("Side Bar User Info"), ImVec2(sideBarContentSize.x, 100), false, ImGuiWindowFlags_NoBackground);
                    {
                        const ImVec2 sideBarUserInfoSize = ImGui::GetCurrentWindow()->Size;
                        const ImVec2 sideBarUserInfoPos = ImGui::GetCurrentWindow()->Pos;

                        ImGui::GetCurrentWindow()->DrawList->AddRectFilled(sideBarUserInfoPos, sideBarUserInfoPos + sideBarUserInfoSize, ImColor(StyleProvider::MenuColourVeryDark), style.ChildRounding);

                        ImGui::GetWindowDrawList()->AddImageRounded(Communication::GetUser()->GetAvatarTexture(), sideBarUserInfoPos + ImVec2(sideBarUserInfoSize.y / 4, sideBarUserInfoSize.y / 4), sideBarUserInfoPos + ImVec2(sideBarUserInfoSize.y / 4 + sideBarUserInfoSize.y / 2, sideBarUserInfoSize.y / 4 + sideBarUserInfoSize.y / 2), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), style.FrameRounding);

                        ImGui::PushFont(StyleProvider::FontDefaultBold);
                        ImGui::SetCursorPos(ImVec2(sideBarUserInfoSize.y / 4 + sideBarUserInfoSize.y / 2 + style.ItemSpacing.x, sideBarUserInfoSize.y / 2 - style.ItemSpacing.y / 4 - ImGui::CalcTextSize("Welcome back").y));
                        ImGui::Text(xorstr_("Welcome back"));

                        ImGui::SetCursorPos(ImVec2(sideBarUserInfoSize.y / 4 + sideBarUserInfoSize.y / 2 + style.ItemSpacing.x, sideBarUserInfoSize.y / 2 + style.ItemSpacing.y / 4));
                        Widgets::Link(Communication::GetUser()->GetUsername().c_str(), "https://maple.software/dashboard", false);
                        ImGui::PopFont();
                    }
                    ImGui::EndChild();

                    ImGui::BeginChild(xorstr_("Side Bar Cheats"), ImVec2(sideBarContentSize.x, sideBarContentSize.y - ImGui::GetCursorPosY()), false, ImGuiWindowFlags_NoBackground);
                    {
                        const ImVec2 sideBarCheatsSize = ImGui::GetCurrentWindow()->Size;
                        const ImVec2 sideBarCheatsPos = ImGui::GetCurrentWindow()->Pos;

                        ImGui::GetCurrentWindow()->DrawList->AddRectFilled(sideBarCheatsPos, sideBarCheatsPos + sideBarCheatsSize, ImColor(StyleProvider::MenuColourVeryDark), style.ChildRounding);

                        ImGui::SetCursorPos(StyleProvider::Padding);
                        ImGui::BeginChild(xorstr_("Side Bar Cheats Content"), sideBarCheatsSize - StyleProvider::Padding * 2, false, ImGuiWindowFlags_NoBackground);
                        {
                            if (shouldSelectFirstGame)
                            {
                                shouldSelectFirstGame = false;
                                ImGui::SetNextTreeNodeOpen(true);
                            }

                            for (unsigned int i = 0; i < Communication::GetAllGames().size(); i++)
                            {
                                ImGui::PushFont(StyleProvider::FontDefaultBold);

                                if (loadingImage)
                                    ImGui::BeginDisabled();

                                const bool nodeOpened = Widgets::TreeNode(Communication::GetAllGames()[i]->GetName().c_str(), Communication::GetAllGames()[i]->GetIconTexture());

                                if (loadingImage)
                                    ImGui::EndDisabled();

                                ImGui::PopFont();

                                if (nodeOpened)
                                {
                                    for (unsigned int j = 0; j < Communication::GetAllCheats().size(); j++)
                                    {
                                        if (Communication::GetAllCheats()[j]->GetGameID() != Communication::GetAllGames()[i]->GetID())
                                            continue;

                                        ImGui::PushFont(StyleProvider::FontDefaultSemiBold);

                                        if (loadingImage)
                                            ImGui::BeginDisabled();

                                        const bool selected = Widgets::Selectable(Communication::GetAllCheats()[j]->GetName().c_str(), Communication::GetSelectedCheat()->GetID() == Communication::GetAllCheats()[j]->GetID(), ImGuiSelectableFlags_SpanAllColumns);

                                        if (loadingImage)
                                            ImGui::EndDisabled();

                                        ImGui::PopFont();

                                        if (selected)
                                        {
                                            Communication::SelectGame(Communication::GetAllCheats()[j]->GetGameID());
                                            Communication::SelectCheat(Communication::GetAllCheats()[j]->GetID());
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
            ImGui::BeginChild(xorstr_("Cheat Banner"), StyleProvider::CheatBannerSize, false, ImGuiWindowFlags_NoBackground);
            {
                const ImVec2 cheatBannerSize = ImGui::GetCurrentWindow()->Size;

                ImGui::Image(Communication::GetSelectedGame()->GetBannerTexture(), cheatBannerSize);
                Widgets::Gradient(ImVec2(0, cheatBannerSize.y), ImVec2(cheatBannerSize.x, 0), StyleProvider::CheatBannerGradientStartColour, StyleProvider::CheatBannerGradientEndColour);

                ImGui::SetCursorPos(StyleProvider::Padding);
                ImGui::BeginChild(xorstr_("Cheat Banner Content"), cheatBannerSize - StyleProvider::Padding * 2, false, ImGuiWindowFlags_NoBackground);
                {
                    const ImVec2 cheatBannerContentSize = ImGui::GetCurrentWindow()->Size;

                    ImGui::PushFont(StyleProvider::FontHugeBold);
                    ImGui::Text(Communication::GetSelectedCheat()->GetName().c_str());
                    ImGui::PopFont();

                    ImGui::SetCursorPosY(ImGui::GetCursorPos().y - ImGui::GetStyle().ItemSpacing.y);
                    ImGui::PushFont(StyleProvider::FontDefaultSemiBold);
                    ImGui::Text(xorstr_("for %s"), Communication::GetSelectedGame()->GetName().c_str());

                    const float priceHeight = ImGui::CalcTextSize(xorstr_("Starts at 5 euro")).y;
                    ImGui::SetCursorPosY(cheatBannerContentSize.y - ImGui::GetFrameHeight() / 2 - priceHeight / 2);
                    ImGui::Text(xorstr_("Starts at %i euro"), Communication::GetSelectedCheat()->GetStartingPrice());
                    ImGui::PopFont();

                    const float widgetWidth = cheatBannerContentSize.x * 0.25f;

                    if (loadingImage)
                        ImGui::BeginDisabled();

                    ImGui::SetCursorPos(cheatBannerContentSize - ImVec2(widgetWidth, ImGui::GetFrameHeight() * 2 + style.ItemSpacing.y));
                    ImGui::PushItemWidth(widgetWidth);
                    Widgets::Combo(xorstr_("###ReleaseStream"), &Communication::GetSelectedCheat()->CurrentStream, Communication::GetSelectedCheat()->GetReleaseStreams());

                    if (loadingImage)
                        ImGui::EndDisabled();

                    ImGui::SetCursorPos(cheatBannerContentSize - ImVec2(widgetWidth, ImGui::GetFrameHeight()));
                    if (strcmp(Communication::GetSelectedCheat()->GetExpiration().c_str(), xorstr_("not subscribed")) == 0)
                    {
                        if (ImGui::Button(xorstr_("Buy now"), ImVec2(widgetWidth, ImGui::GetFrameHeight())))
                            ShellExecuteA(NULL, xorstr_("open"), xorstr_("https://maple.software/dashboard/store"), NULL, NULL, SW_SHOWNORMAL);
                    }
                    else
                    {
                        if (loadingImage)
                            ImGui::BeginDisabled();

                        const bool loadClicked = Widgets::Button(Communication::GetState() == States::LoadingPayload ? xorstr_("Loading...") : xorstr_("Load"), ImVec2(widgetWidth, ImGui::GetFrameHeight()));

                        if (loadingImage)
                            ImGui::EndDisabled();

                        if (loadClicked)
                        {
                            Communication::RequestLoader();
                        }
                    }
                }
                ImGui::EndChild();
            }
            ImGui::EndChild();

            ImGui::SetCursorPos(ImVec2(StyleProvider::MainWindowSideBarSize.x, StyleProvider::CheatBannerSize.y));
            ImGui::BeginChild(xorstr_("Cheat Info"), ImVec2(mainWindowSize.x - StyleProvider::MainWindowSideBarSize.x, mainWindowSize.y - StyleProvider::CheatBannerSize.y), false, ImGuiWindowFlags_NoBackground);
            {
                const ImVec2 cheatInfoSize = ImGui::GetCurrentWindow()->Size;

                ImGui::SetCursorPos(StyleProvider::Padding);
                ImGui::BeginChild(xorstr_("Cheat Info Content"), cheatInfoSize - StyleProvider::Padding * 2, false, ImGuiWindowFlags_NoBackground);
                {
                    ImGui::PushFont(StyleProvider::FontBigBold);
                    ImGui::Text(xorstr_("Subscription expires on"));
                    ImGui::PopFont();

                    ImGui::PushFont(StyleProvider::FontDefaultSemiBold);
                    ImGui::Text(Communication::GetSelectedCheat()->GetExpiration().c_str());
                    if (strcmp(Communication::GetSelectedCheat()->GetExpiration().c_str(), xorstr_("not subscribed")) == 0)
                    {
                        ImGui::SameLine();
                        Widgets::Link(xorstr_("subscribe now!"), xorstr_("https://maple.software/dashboard/store"), false);
                    }
                    ImGui::PopFont();

                    ImGui::Spacing();

                    ImGui::PushFont(StyleProvider::FontBigBold);
                    ImGui::Text(xorstr_("Status"));
                    ImGui::PopFont();

                    ImGui::PushFont(StyleProvider::FontDefaultSemiBold);
                    switch (Communication::GetSelectedCheat()->GetStatus())
                    {
	                    case CheatStatus::Undetected:
	                        ImGui::TextColored(ImVec4(0, 1, 0, 1), xorstr_("undetected"));
	                        break;
	                    case CheatStatus::Outdated:
	                        ImGui::TextColored(ImVec4(ImColor(255, 69, 0, 255)), xorstr_("outdated"));
	                        break;
	                    case CheatStatus::Detected:
	                        ImGui::TextColored(ImVec4(1, 0, 0, 1), xorstr_("detected"));
	                        break;
						case CheatStatus::Unknown:
                            ImGui::Text(xorstr_("unknown"));
                            break;
                    }
                    ImGui::PopFont();

                    ImGui::Spacing();

                    ImGui::PushFont(StyleProvider::FontDefaultSemiBold);
                    Widgets::Link(xorstr_("Visit our website for more information"), xorstr_("https://maple.software/"), false);
                    ImGui::PopFont();
                }
                ImGui::EndChild();
            }
            ImGui::EndChild();
        }

        ImGui::SetCursorPos(ImVec2(mainWindowSize.x - StyleProvider::WindowControlSize.x * 2, 0));
        ImGui::BeginChild(xorstr_("Window Controls"), ImVec2(StyleProvider::WindowControlSize.x * 2, StyleProvider::WindowControlSize.y), false, ImGuiWindowFlags_NoBackground);
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            ImGui::PushStyleColor(ImGuiCol_Text, StyleProvider::WindowControlTextColour);

            ImGui::PushStyleColor(ImGuiCol_Button, StyleProvider::MinimizeButtonColour);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, StyleProvider::MinimizeButtonHoveredColour);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, StyleProvider::MinimizeButtonActiveColour);

            if (Widgets::Button(xorstr_("_"), StyleProvider::WindowControlSize))
                ShowWindow(NativeWindow, SW_MINIMIZE);

            ImGui::PopStyleColor(3);

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, StyleProvider::CloseButtonColour);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, StyleProvider::CloseButtonHoveredColour);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, StyleProvider::CloseButtonActiveColour);

            if (Widgets::Button(xorstr_("x"), StyleProvider::WindowControlSize))
                glfwSetWindowShouldClose(GLFWWindow, 1);

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);
        }
        ImGui::EndChild();

        ImGui::PopFont();
    }
    ImGui::End();

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(GLFWWindow, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(GLFWWindow);

    if (loggedIn && inLoginState)
    {
        setWindowSize(static_cast<int>(StyleProvider::MainWindowSize.x), static_cast<int>(StyleProvider::MainWindowSize.y));

        inLoginState = false;
    }
    else if (!loggedIn && !inLoginState)
    {
        setWindowSize(static_cast<int>(StyleProvider::LoginWindowSize.x), static_cast<int>(StyleProvider::LoginWindowSize.y));

        inLoginState = true;
    }
}

void UI::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(GLFWWindow);
    glfwTerminate();

    StyleProvider::Shutdown();
}
