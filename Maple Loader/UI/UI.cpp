#include "UI.h"

#include "../Globals.h"
#include "../resource.h"

#include "ImGui/imgui_impl_dx9.h"
#include "ImGui/imgui_impl_win32.h"

#include "StyleProvider.h"
#include "Widgets.h"
#include "AnimationHandler.h"

#include "../AntiDebug/xorstr.hpp"
#include "../Packets/Requests/DllStreamRequest.h"
#include "../Packets/Requests/LoginRequest.h"
#include "../Utils/HWID.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT UI::wndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wparam, lparam))
		return true;

	if (message == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wparam, lparam);
}

HWND UI::createWindow(HINSTANCE instance)
{
	WNDCLASSEX wc;
	memset(&wc, 0, sizeof(wc));

	HICON hIcon = static_cast<HICON>(::LoadImage(instance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR));
	
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = wndProc;
	wc.hInstance = instance;
	wc.hIcon = hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = 0;
	wc.lpszClassName = L"LoaderClass";

	RECT pos;
	GetClientRect(GetDesktopWindow(), &pos);
	pos.left = (pos.right / 2) - (StyleProvider::WindowSize.x / 2);
	pos.top = (pos.bottom / 2) - (StyleProvider::WindowSize.y / 2);

	RegisterClassEx(&wc);

	return CreateWindowEx(0, wc.lpszClassName, L"Maple Loader", WS_POPUP, pos.left, pos.top, StyleProvider::WindowSize.x, StyleProvider::WindowSize.y, 0, 0, wc.hInstance, 0);
}

bool UI::createDevice(HWND hwnd)
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!d3d)
		return false;

	memset(&D3DPresentParams, 0, sizeof(D3DPresentParams));

	D3DPresentParams.Windowed = TRUE;
	D3DPresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	D3DPresentParams.BackBufferFormat = D3DFMT_UNKNOWN;
	D3DPresentParams.EnableAutoDepthStencil = TRUE;
	D3DPresentParams.AutoDepthStencilFormat = D3DFMT_D16;
	D3DPresentParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	auto res = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &D3DPresentParams, &D3DDevice);
	if (res != D3D_OK)
		return false;

	return true;
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
	}

	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !ImGui::GetIO().WantTextInput)
	{
		POINT point;
		GetCursorPos(&point);

		SetWindowPos(Window, nullptr, point.x - dragOffsetX, point.y - dragOffsetY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
}

bool UI::Initialize(HINSTANCE instance, int showCmd)
{
	Window = createWindow(instance);
	if (!Window)
		return false;

	if (!createDevice(Window))
		return false;

	ShowWindow(Window, showCmd);

	SetWindowRgn(Window, CreateRoundRectRgn(0, 0, 600, 400, 20, 20), true); //rounded corners uwu

	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	StyleProvider::LoadFonts();
	StyleProvider::LoadColours();
	StyleProvider::LoadTextures();
	
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowPadding = ImVec2(0, 0);
	style.WindowBorderSize = 0;
	style.ItemSpacing = ImVec2(5, 10);
	style.FrameRounding = 5;
	style.FramePadding = ImVec2(5, 5);
	style.ChildRounding = 20;

	ImGui_ImplWin32_Init(Window);
	ImGui_ImplDX9_Init(D3DDevice);

	return true;
}

bool UI::Render()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	handleWindowDrag();

	ImGui::SetNextWindowSize(StyleProvider::WindowSize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2{ 0, 0 }, ImGuiCond_Always);

	ImGui::Begin(xor ("MainWindow"), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
	{
		ImGui::PushFont(StyleProvider::FontDefault);

		if (Globals::LoaderState < LoaderStates::LoggedIn)
			AnimationHandler::DoAnimation(&StyleProvider::LoginBackgroundTextureAnimated, ImVec2(0, 0), StyleProvider::WindowSize);
			//ImGui::GetWindowDrawList()->AddImage(StyleProvider::LoginBackgroundTexture, ImVec2(0, 0), StyleProvider::WindowSize);

		if (Globals::LoaderState < LoaderStates::LoggedIn)
		{
			ImGui::SetCursorPos(ImVec2(StyleProvider::LoginBackgroundWavesWidth + StyleProvider::LoginPadding.x, StyleProvider::LoginPadding.y));
			ImGui::BeginChild(xor ("Login"), ImVec2(StyleProvider::WindowSize.x - StyleProvider::LoginBackgroundWavesWidth - StyleProvider::LoginPadding.x * 2, StyleProvider::WindowSize.y - StyleProvider::LoginPadding.y * 2));
			{
				ImGui::SetCursorPos(ImGui::GetCursorPos() + StyleProvider::LoginPanelPadding);
				ImGui::BeginChild(xor ("Login##001"), ImGui::GetWindowSize() - StyleProvider::LoginPanelPadding * 2);
				{
					ImGui::PushFont(StyleProvider::FontHugeBold);
					ImVec2 welcomeSize = ImGui::CalcTextSize(xor ("Welcome to Maple!"));
					ImGui::SetCursorPosX(ImGui::GetWindowSize().x / 2 - welcomeSize.x / 2);
					ImGui::Text(xor ("Welcome to Maple!"));
					ImGui::PopFont();

					ImGui::PushFont(StyleProvider::FontSmallSemiBold);
					ImVec2 mottoSize = ImGui::CalcTextSize(xor ("the quickest way to the top"));
					ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2 - mottoSize.x / 2, welcomeSize.y));
					ImGui::TextColored(StyleProvider::MottoColour, xor ("the quickest way to the top"));
					ImGui::PopFont();

					ImGui::Spacing();

					bool loggingIn = Globals::LoaderState == LoaderStates::LoggingIn;

					ImGui::Text(xor ("Username"));
					ImGui::PushItemWidth(ImGui::GetWindowSize().x);
					if (loggingIn)
					{
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
						ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
					}
					Widgets::TextBox(xor ("##username"), Globals::CurrentUser.Username, 24);
					if (loggingIn)
					{
						ImGui::PopStyleVar();
						ImGui::PopItemFlag();
					}
					ImGui::Spacing();

					ImGui::Text(xor ("Password"));
					ImGui::PushItemWidth(ImGui::GetWindowSize().x);
					if (loggingIn)
					{
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
						ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
					}
					Widgets::TextBox(xor ("##password"), Globals::CurrentUser.Password, 256, ImGuiInputTextFlags_Password);
					if (loggingIn)
					{
						ImGui::PopStyleVar();
						ImGui::PopItemFlag();
					}
					Widgets::Link(xor ("Forgot your password?"), xor ("https://maple.software/auth/forgotPassword"));

					ImGui::Spacing();

					if (loggingIn)
					{
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
						ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
					}
					if (ImGui::Button(xor(Globals::LoaderState == LoaderStates::LoggingIn ? "Logging in..." : "Log in"), ImVec2(ImGui::GetWindowSize().x, ImGui::GetFrameHeight())))
					{
						std::string hwid = HWID::GetHWID();
						if (hwid.empty())
						{
							MessageBoxA(Window, xor ("Failed to gather hardware information!\nPlease report this.\n\nThe application will now exit."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);

							Globals::ShutdownAndExit();
						}
						
						LoginRequest loginPacket = LoginRequest(hwid, Globals::CurrentUser.Username, Globals::CurrentUser.Password, Globals::MatchedClient);

						pipe_ret_t sendRet = Globals::TCPClient.sendBytes(loginPacket.Data);
						if (!sendRet.success)
						{
							MessageBoxA(Window, xor ("Failed to communicate with the server!\nThe application will now exit."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);

							Globals::ShutdownAndExit();
						}

						Globals::LoaderState = LoaderStates::LoggingIn;
					}
					if (loggingIn)
					{
						ImGui::PopStyleVar();
						ImGui::PopItemFlag();
					}
					
					Widgets::LinkWithText(xor ("Sign up"), xor ("https://maple.software/auth/signup"), xor ("Need an account?"), StyleProvider::MottoColour);
				}
				ImGui::EndChild();
			}
			ImGui::EndChild();
		}
		else
		{
			bool loadingDll = Globals::LoaderState == LoaderStates::LoadingDLL;
			
			ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(-20, -20), StyleProvider::LoaderSideBarSize, ImColor(ImGui::GetStyle().Colors[ImGuiCol_ChildBg]), 20);
			ImGui::SetCursorPos(StyleProvider::LoaderPadding);
			ImGui::BeginChild(xor ("LoaderSideBar"), StyleProvider::LoaderSideBarSize - StyleProvider::LoaderPadding * 2);
			{
				ImGui::PushFont(StyleProvider::FontHugeBold);
				ImVec2 titleSize = ImGui::CalcTextSize(xor ("Maple"));
				ImGui::SetCursorPosX(ImGui::GetWindowSize().x / 2 - titleSize.x / 2 - ImGui::GetStyle().ItemSpacing.x - (StyleProvider::MapleLogoSize.x / 2));
				ImGui::Image(StyleProvider::MapleLogoTexture, StyleProvider::MapleLogoSize);
				ImGui::SameLine();
				ImGui::TextColored(StyleProvider::LinkColour, xor ("Maple"));
				ImGui::PopFont();

				ImGui::PushFont(StyleProvider::FontSmallSemiBold);
				ImVec2 mottoSize = ImGui::CalcTextSize(xor ("the quickest way to the top"));
				ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2 - mottoSize.x / 2, titleSize.y));
				ImGui::TextColored(StyleProvider::MottoColour, xor ("the quickest way to the top"));
				ImGui::PopFont();

				ImGui::Spacing();

				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(ImColor(38, 38, 38, 255)));
				ImGui::BeginChild(xor ("LoaderSideBar#001"), ImVec2(ImGui::GetWindowSize().x, 50));
				{
					ImGui::PopStyleColor();

					ImVec2 loggedInAsSize = ImGui::CalcTextSize(xor ("logged in as"));
					ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2 - loggedInAsSize.x / 2, ImGui::GetWindowSize().y / 2 - loggedInAsSize.y - 2));
					ImGui::Text(xor ("logged in as"));

					ImVec2 usernameSize = ImGui::CalcTextSize(Globals::CurrentUser.Username);
					ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2 - usernameSize.x / 2, ImGui::GetWindowSize().y / 2 + 2));
					Widgets::Link(Globals::CurrentUser.Username, xor ("https://maple.software/dashboard"), false);
				}
				ImGui::EndChild();

				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(ImColor(38, 38, 38, 255)));
				ImGui::BeginChild(xor ("LoaderSideBar#002"), ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y - ImGui::GetCursorPosY()));
				{
					ImGui::SetCursorPos(ImVec2(StyleProvider::LoaderPadding));
					ImGui::BeginChild(xor ("LoaderSideBar##003"), ImGui::GetWindowSize() - StyleProvider::LoaderPadding * 2);
					ImGui::PopStyleColor();
					if (shouldSelectFirstGame)
					{
						shouldSelectFirstGame = false;
						ImGui::SetNextTreeNodeOpen(true);
					}
					for (int i = 0; i < Globals::Games.size(); i++)
					{
						if (loadingDll)
						{
							ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
							ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
						}
						bool nodeOpen = Widgets::TreeNode(Globals::Games[i]->Name.c_str(), Globals::Games[i]->Icon);
						if (loadingDll)
						{
							ImGui::PopStyleVar();
							ImGui::PopItemFlag();
						}
						if (nodeOpen)
						{
							for (int j = 0; j < Globals::Cheats.size(); j++)
							{
								if (Globals::Cheats[j]->GameID != Globals::Games[i]->ID)
									continue;

								if (loadingDll)
								{
									ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
									ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
								}
								bool selected = ImGui::Selectable(Globals::Cheats[j]->Name.c_str(), Globals::CurrentCheat->ID == Globals::Cheats[j]->ID, 0, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorPosX() - ceil(ImGui::GetStyle().FramePadding.x / 2.0f), ImGui::GetFont()->FontSize));
								if (loadingDll)
								{
									ImGui::PopStyleVar();
									ImGui::PopItemFlag();
								}
								if (selected)
								{
									Globals::CurrentCheat = Globals::Cheats[j];

									for (Game* game : Globals::Games)
										if (game->ID == Globals::CurrentCheat->GameID)
											Globals::CurrentGame = game;
								}
							}

							ImGui::TreePop();
						}
					}
					ImGui::EndChild();
				}
				ImGui::EndChild();
			}
			ImGui::EndChild();

			ImGui::SetCursorPos(ImVec2(StyleProvider::LoaderSideBarSize.x, 0));
			ImGui::BeginChild(xor ("CheatBanner"), StyleProvider::CheatBannerSize, false, ImGuiWindowFlags_NoBackground);
			{
				ImGui::GetWindowDrawList()->AddImage(Globals::CurrentGame->Banner, ImVec2(StyleProvider::LoaderSideBarSize.x, 0), ImVec2(StyleProvider::LoaderSideBarSize.x, 0) + ImGui::GetWindowSize());
				Widgets::Gradient(ImVec2(StyleProvider::LoaderSideBarSize.x, ImGui::GetWindowSize().y), ImVec2(StyleProvider::LoaderSideBarSize.x + ImGui::GetWindowSize().x, 0), StyleProvider::BannerGradientStartColour, StyleProvider::BannerGradientEndColour);

				ImGui::SetCursorPos(StyleProvider::LoaderPadding);
				ImGui::BeginChild(xor ("CheatBanner##001"), ImGui::GetWindowSize() - StyleProvider::LoaderPadding * 2, false, ImGuiWindowFlags_NoBackground);
				{
					ImGui::PushFont(StyleProvider::FontHugeBold);
					ImGui::Text(Globals::CurrentCheat->Name.c_str());
					ImGui::PopFont();

					ImGui::SetCursorPosY(ImGui::GetCursorPos().y - ImGui::GetStyle().ItemSpacing.y);
					ImGui::PushFont(StyleProvider::FontDefaultSemiBold);
					ImGui::Text(xor ("for %s"), Globals::CurrentGame->Name.c_str());

					ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - 100, ImGui::GetWindowSize().y - ImGui::GetFrameHeight() * 2 - ImGui::GetStyle().ItemSpacing.y));
					ImGui::PushItemWidth(100);
					if (loadingDll)
					{
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
						ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
					}
					ImGui::Combo("##releaseStream", &Globals::CurrentCheat->CurrentStream, [](void* vec, int idx, const char** out_text)
					{
						auto& vector = *static_cast<std::vector<std::string>*>(vec);
						if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
						*out_text = vector.at(idx).c_str();
						return true;
					}, reinterpret_cast<void*>(&Globals::CurrentCheat->ReleaseStreams), Globals::CurrentCheat->ReleaseStreams.size());
					if (loadingDll)
					{
						ImGui::PopStyleVar();
						ImGui::PopItemFlag();
					}
					
					ImVec2 buttonSize = ImVec2(100, ImGui::GetFrameHeight());
					ImGui::SetCursorPos(ImGui::GetWindowSize() - buttonSize);
					if (strcmp(Globals::CurrentCheat->ExpiresAt.c_str(), xor ("not subscribed")) == 0)
					{
						if (ImGui::Button(xor ("Buy now"), buttonSize))
							ShellExecuteA(NULL, xor("open"), xor("https://maple.software/dashboard/store"), NULL, NULL, SW_SHOWNORMAL);
					}
					else
					{
						if (loadingDll)
						{
							ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
							ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
						}
						bool loadClicked = ImGui::Button(xor (Globals::LoaderState == LoaderStates::LoadingDLL ? "Loading..." : "Load"), buttonSize);
						if (loadingDll)
						{
							ImGui::PopStyleVar();
							ImGui::PopItemFlag();
						}
						if (loadClicked)
						{
							DllStreamRequest dllStream = DllStreamRequest(Globals::CurrentCheat->ID, Globals::CurrentCheat->ReleaseStreams[Globals::CurrentCheat->CurrentStream], Globals::MatchedClient);

							pipe_ret_t sendRet = Globals::TCPClient.sendBytes(dllStream.Data);
							if (!sendRet.success)
							{
								MessageBoxA(Window, xor ("Failed to communicate with the server!\nThe application will now exit."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);

								Globals::ShutdownAndExit();
							}

							Globals::LoaderState = LoaderStates::LoadingDLL;
						}
					}

					ImVec2 priceSize = ImGui::CalcTextSize(xor ("10 euro per month"));
					ImGui::SetCursorPosY(ImGui::GetWindowSize().y - ImGui::GetFrameHeight() / 2 - priceSize.y / 2);
					ImGui::Text(xor ("%i euro per month"), Globals::CurrentCheat->Price);
					ImGui::PopFont();
				}
				ImGui::EndChild();
			}
			ImGui::EndChild();

			ImGui::SetCursorPos(ImVec2(StyleProvider::LoaderSideBarSize.x, StyleProvider::CheatBannerSize.y));
			ImGui::BeginChild(xor ("CheatInfo"), ImVec2(StyleProvider::WindowSize.x - StyleProvider::LoaderSideBarSize.x, StyleProvider::WindowSize.y - StyleProvider::CheatBannerSize.y), false, ImGuiWindowFlags_NoBackground);
			{
				ImGui::SetCursorPos(StyleProvider::LoaderPadding);
				ImGui::BeginChild("CheatInfo#001", ImGui::GetWindowSize() - StyleProvider::LoaderPadding * 2, false, ImGuiWindowFlags_NoBackground);
				{
					ImGui::PushFont(StyleProvider::FontBigBold);
					ImGui::Text(xor ("Subscription expires at"));
					ImGui::PopFont();

					ImGui::PushFont(StyleProvider::FontDefaultSemiBold);
					ImGui::Text(Globals::CurrentCheat->ExpiresAt.c_str());
					if (strcmp(Globals::CurrentCheat->ExpiresAt.c_str(), xor ("not subscribed")) == 0)
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
					switch (Globals::CurrentCheat->Status)
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
					ImGui::Text(Globals::CurrentCheat->Features.c_str());
					Widgets::Link(xor ("Visit our website for a full list of features"), xor ("https://maple.software/"), false);
					ImGui::PopFont();
				}
				ImGui::EndChild();
			}
			ImGui::EndChild();
		}

		ImGui::SetCursorPos(ImVec2(StyleProvider::WindowSize.x - StyleProvider::WindowControlSize.x * 2, 0));
		ImGui::BeginChild(xor("WindowControls"), ImVec2(StyleProvider::WindowControlSize.x * 2, StyleProvider::WindowControlSize.y), false, ImGuiWindowFlags_NoBackground);
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			ImGui::PushStyleColor(ImGuiCol_Text, StyleProvider::WindowControlTextColour);

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(0, 0, 0, 0)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, StyleProvider::MinimizeButtonHoveredColour);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, StyleProvider::MinimizeButtonActiveColour);

			if (ImGui::Button(xor ("_"), StyleProvider::WindowControlSize))
				ShowWindow(Window, SW_MINIMIZE);
			ImGui::PopStyleColor(3);

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(0, 0, 0, 0)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, StyleProvider::CloseButtonHoveredColour);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, StyleProvider::CloseButtonActiveColour);
			if (ImGui::Button(xor ("x"), StyleProvider::WindowControlSize))
				Globals::ShutdownAndExit();

			ImGui::PopStyleVar();
			ImGui::PopStyleColor(4);
		}
		ImGui::EndChild();

		ImGui::PopFont();
	}
	ImGui::End();

	ImGui::EndFrame();
	if (D3DDevice->BeginScene() == D3D_OK)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		D3DDevice->EndScene();
	}

	HRESULT result = D3DDevice->Present(0, 0, 0, 0);
	if (result == D3DERR_DEVICELOST && D3DDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		HRESULT hr = D3DDevice->Reset(&D3DPresentParams);
		if (hr == D3DERR_INVALIDCALL)
			return false;

		ImGui_ImplDX9_CreateDeviceObjects();
	}

	return true;
}

void UI::Shutdown()
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	
	D3DDevice->Release();
	d3d->Release();
	
	DestroyWindow(Window);
}
