#include "StyleProvider.h"

#include <string>

#include "../Utilities/Textures/TextureHelper.h"
#include "Textures.h"
#include "UI.h"
#include "Fonts.h"

void StyleProvider::loadFonts()
{
	ImGuiIO& io = ImGui::GetIO();

	FontDefault = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaData, Fonts::ComfortaaSize, 18);
	FontDefaultSemiBold = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaSemiBoldData, Fonts::ComfortaaSemiBoldSize, 18);
	FontDefaultBold = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaBoldData, Fonts::ComfortaaBoldSize, 18);

	FontSmall = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaData, Fonts::ComfortaaSize, 16);
	FontSmallSemiBold = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaSemiBoldData, Fonts::ComfortaaSemiBoldSize, 16);
	FontSmallBold = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaBoldData, Fonts::ComfortaaBoldSize, 16);

	FontBig = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaData, Fonts::ComfortaaSize, 24);
	FontBigSemiBold = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaSemiBoldData, Fonts::ComfortaaSemiBoldSize, 24);
	FontBigBold = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaBoldData, Fonts::ComfortaaBoldSize, 24);

	FontHuge = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaData, Fonts::ComfortaaSize, 28);
	FontHugeSemiBold = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaSemiBoldData, Fonts::ComfortaaSemiBoldSize, 28);
	FontHugeBold = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaBoldData, Fonts::ComfortaaBoldSize, 28);
}

void StyleProvider::loadColours()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_TextSelectedBg] = AccentColour;

	style.Colors[ImGuiCol_FrameBg] = ControlColour;
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(ControlColour.x + 0.05f, ControlColour.y + 0.05f, ControlColour.z + 0.05f, ControlColour.w);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(ControlColour.x - 0.05f, ControlColour.y - 0.05f, ControlColour.z - 0.05f, ControlColour.w);

	MottoColour = ImVec4(MenuColour.x + 0.2f, MenuColour.y + 0.2f, MenuColour.z + 0.2f, MenuColour.w);
	MenuColourDark = ImVec4(MenuColour.x - 0.05f, MenuColour.y - 0.05f, MenuColour.z - 0.05f, MenuColour.w);
	MenuColourVeryDark = ImVec4(MenuColour.x - 0.1f, MenuColour.y - 0.1f, MenuColour.z - 0.1f, MenuColour.w);

	LinkColour = AccentColour;
	LinkHoveredColour = ImVec4(LinkColour.x + 0.05f, LinkColour.y + 0.05f, LinkColour.z + 0.05f, LinkColour.w);
	LinkActiveColour = ImVec4(LinkColour.x - 0.05f, LinkColour.y - 0.05f, LinkColour.z - 0.05f, LinkColour.w);

	CheatBannerGradientStartColour = MenuColour;
	CheatBannerGradientEndColour = ImVec4(CheatBannerGradientStartColour.x, CheatBannerGradientStartColour.y, CheatBannerGradientStartColour.z, 75.0f / 255.0f);

	WindowControlTextColour = ImVec4(MottoColour.x + 0.25f, MottoColour.y + 0.25f, MottoColour.z + 0.25f, MottoColour.w);
	MinimizeButtonColour = ImVec4(MenuColourDark.x + 0.1f, MenuColourDark.y + 0.1f, MenuColourDark.z + 0.1f, 0.f);
	MinimizeButtonHoveredColour = ImVec4(MinimizeButtonColour.x + 0.1f, MinimizeButtonColour.y + 0.1f, MinimizeButtonColour.z + 0.1f, 150.0f / 255.0f);
	MinimizeButtonActiveColour = ImVec4(MinimizeButtonHoveredColour.x - 0.05f, MinimizeButtonHoveredColour.y - 0.05f, MinimizeButtonHoveredColour.z - 0.05f, 150.0f / 255.0f);
	CloseButtonColour = ImVec4(ImColor(178, 0, 0, 0));
	CloseButtonHoveredColour = ImVec4(ImColor(178, 0, 0, 150));
	CloseButtonActiveColour = ImVec4(CloseButtonHoveredColour.x - 0.05f, CloseButtonHoveredColour.y - 0.05f, CloseButtonHoveredColour.z - 0.05f, CloseButtonHoveredColour.w);

	style.Colors[ImGuiCol_WindowBg] = MenuColour;
	style.Colors[ImGuiCol_ChildBg] = MenuColourDark;

	style.Colors[ImGuiCol_PopupBg] = MenuColour;

	style.Colors[ImGuiCol_TitleBg] = MenuColourVeryDark;
	style.Colors[ImGuiCol_TitleBgActive] = MenuColourVeryDark;
	style.Colors[ImGuiCol_TitleBgCollapsed] = MenuColourVeryDark;

	style.Colors[ImGuiCol_Header] = style.Colors[ImGuiCol_FrameBg];
	style.Colors[ImGuiCol_HeaderHovered] = style.Colors[ImGuiCol_FrameBgHovered];
	style.Colors[ImGuiCol_HeaderActive] = style.Colors[ImGuiCol_FrameBgActive];

	style.Colors[ImGuiCol_Button] = style.Colors[ImGuiCol_FrameBg];
	style.Colors[ImGuiCol_ButtonHovered] = style.Colors[ImGuiCol_FrameBgHovered];
	style.Colors[ImGuiCol_ButtonActive] = style.Colors[ImGuiCol_FrameBgActive];
}

void StyleProvider::loadTextures()
{
	MapleLogoTexture = TextureHelper::CreateTexture(Textures::MapleLogo, Textures::MapleLogoSize);
	ChevronIconTexture = TextureHelper::CreateTexture(Textures::ChevronIcon, Textures::ChevronIconSize);
}

void StyleProvider::Initialize()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.WindowPadding = ImVec2(0, 0);
	style.WindowBorderSize = 0;
	style.ItemSpacing = ImVec2(10, 10);
	style.WindowRounding = 20;
	style.ChildRounding = 20;
	style.FrameRounding = 10;
	style.PopupRounding = 10;
	style.GrabRounding = 10;
	style.FramePadding = ImVec2(5, 5);
	style.PopupBorderSize = 0;

	loadFonts();
	loadColours();
	loadTextures();
}
