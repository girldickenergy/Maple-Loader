#pragma once

#include "ImGui/imgui.h"

#include "Fonts.h"
#include "Textures.h"

class StyleProvider
{
public:
	static inline ImVec2 WindowSize = ImVec2(600, 400);

	//fonts
	static inline ImFont* FontDefault;
	static inline ImFont* FontDefaultSemiBold;
	static inline ImFont* FontDefaultBold;
	static inline ImFont* FontSmall;
	static inline ImFont* FontSmallSemiBold;
	static inline ImFont* FontSmallBold;
	static inline ImFont* FontBig;
	static inline ImFont* FontBigSemiBold;
	static inline ImFont* FontBigBold;
	static inline ImFont* FontHuge;
	static inline ImFont* FontHugeSemiBold;
	static inline ImFont* FontHugeBold;

	//main colours
	static inline ImVec4 WindowColour = ImVec4(ImColor(65, 65, 65, 255));
	static inline ImVec4 ButtonColour = ImVec4(ImColor(71, 176, 229, 255));

	static inline ImVec4 MottoColour;

	//window controls
	static inline ImVec4 WindowControlTextColour;
	static inline ImVec4 MinimizeButtonHoveredColour;
	static inline ImVec4 MinimizeButtonActiveColour;
	static inline ImVec4 CloseButtonHoveredColour;
	static inline ImVec4 CloseButtonActiveColour;

	//links
	static inline ImVec4 LinkColour;
	static inline ImVec4 LinkHoveredColour;
	static inline ImVec4 LinkActiveColour;

	//controls
	static inline ImVec4 DropDownArrowColour;
	static inline ImVec4 DropDownArrowHoveredColour;

	//gradient
	static inline ImVec4 BannerGradientStartColour;
	static inline ImVec4 BannerGradientEndColour;

	//style variables
	static inline ImVec2 LoginPadding = ImVec2(40, 40);
	static inline ImVec2 LoginPanelPadding = ImVec2(20, 20);
	static inline ImVec2 WindowControlSize = ImVec2(50, 30);
	static inline ImVec2 LoaderSideBarSize = ImVec2(200, 400);
	static inline ImVec2 LoaderPadding = ImVec2(10, 10);

	//textures/texture sizes
	static inline float LoginBackgroundWavesWidth = 225;
	static inline IDirect3DTexture9* LoginBackgroundTexture;
	static inline ImVec2 MapleLogoSize = ImVec2(24, 24);
	static inline IDirect3DTexture9* MapleLogoTexture;
	static inline ImVec2 CheatBannerSize = ImVec2(400, 150);

	static inline IDirect3DTexture9* WhitePixelTexture;

	static void LoadFonts()
	{
		ImGuiIO& io = ImGui::GetIO();

		FontDefault = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaData, Fonts::ComfortaaSize, 16);
		FontDefaultSemiBold = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaSemiBoldData, Fonts::ComfortaaSemiBoldSize, 16);
		FontDefaultBold = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaBoldData, Fonts::ComfortaaBoldSize, 16);

		FontSmall = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaData, Fonts::ComfortaaSize, 14);
		FontSmallSemiBold = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaSemiBoldData, Fonts::ComfortaaSemiBoldSize, 14);
		FontSmallBold = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaBoldData, Fonts::ComfortaaBoldSize, 14);

		FontBig = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaData, Fonts::ComfortaaSize, 22);
		FontBigSemiBold = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaSemiBoldData, Fonts::ComfortaaSemiBoldSize, 22);
		FontBigBold = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaBoldData, Fonts::ComfortaaBoldSize, 22);

		FontHuge = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaData, Fonts::ComfortaaSize, 26);
		FontHugeSemiBold = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaSemiBoldData, Fonts::ComfortaaSemiBoldSize, 26);
		FontHugeBold = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaBoldData, Fonts::ComfortaaBoldSize, 26);
	}

	static void LoadColours()
	{
		ImGuiStyle& style = ImGui::GetStyle();

		MottoColour = ImVec4(WindowColour.x + 0.25f, WindowColour.y + 0.25f, WindowColour.z + 0.25f, WindowColour.w);
		
		WindowControlTextColour = ImVec4(MottoColour.x + 0.25f, MottoColour.y + 0.25f, MottoColour.z + 0.25f, MottoColour.w);
		MinimizeButtonHoveredColour = ImVec4(WindowColour.x + 0.1f, WindowColour.y + 0.1f, WindowColour.z + 0.1f, 150.0f / 255.0f);
		MinimizeButtonActiveColour = ImVec4(MinimizeButtonHoveredColour.x - 0.05f, MinimizeButtonHoveredColour.y - 0.05f, MinimizeButtonHoveredColour.z - 0.05f, 150.0f / 255.0f);
		CloseButtonHoveredColour = ImVec4(ImColor(178, 0, 0, 150));
		CloseButtonActiveColour = ImVec4(CloseButtonHoveredColour.x - 0.05f, CloseButtonHoveredColour.y - 0.05f, CloseButtonHoveredColour.z - 0.05f, CloseButtonHoveredColour.w);

		LinkColour = ImVec4(ImColor(232, 93, 155, 255));
		LinkHoveredColour = ImVec4(LinkColour.x + 0.05f, LinkColour.y + 0.05f, LinkColour.z + 0.05f, LinkColour.w);
		LinkActiveColour = ImVec4(LinkColour.x - 0.05f, LinkColour.y - 0.05f, LinkColour.z - 0.05f, LinkColour.w);
		
		style.Colors[ImGuiCol_WindowBg] = WindowColour;
		style.Colors[ImGuiCol_ChildBg] = ImVec4(WindowColour.x - 0.05f, WindowColour.y - 0.05f, WindowColour.z - 0.05f, WindowColour.w);

		style.Colors[ImGuiCol_Button] = ButtonColour;
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(ButtonColour.x + 0.05f, ButtonColour.y + 0.05f, ButtonColour.z + 0.05f, ButtonColour.w);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(ButtonColour.x - 0.05f, ButtonColour.y - 0.05f, ButtonColour.z - 0.05f, ButtonColour.w);

		style.Colors[ImGuiCol_Border] = ImVec4(WindowColour.x - 0.1f, WindowColour.y - 0.1f, WindowColour.z - 0.1f, WindowColour.w);

		style.Colors[ImGuiCol_FrameBg] = ImVec4(WindowColour.x + 0.05f, WindowColour.y + 0.05f, WindowColour.z + 0.05f, WindowColour.w);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(WindowColour.x + 0.1f, WindowColour.y + 0.1f, WindowColour.z + 0.1f, WindowColour.w);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(WindowColour.x, WindowColour.y, WindowColour.z, WindowColour.w);
		
		style.Colors[ImGuiCol_Header] = style.Colors[ImGuiCol_FrameBg];
		style.Colors[ImGuiCol_HeaderHovered] = style.Colors[ImGuiCol_FrameBgHovered];
		style.Colors[ImGuiCol_HeaderActive] = style.Colors[ImGuiCol_FrameBgActive];

		BannerGradientStartColour = WindowColour;
		BannerGradientEndColour = ImVec4(BannerGradientStartColour.x, BannerGradientStartColour.y, BannerGradientStartColour.z, 75.0f / 255.0f);
	}

	static void LoadTextures()
	{
		LoginBackgroundTexture = TextureUtils::CreateTexture(Textures::LoginBackground, Textures::LoginBackgroundSize);
		MapleLogoTexture = TextureUtils::CreateTexture(Textures::MapleLogo, Textures::MapleLogoSize);
		WhitePixelTexture = TextureUtils::CreateTexture(Textures::WhitePixel, Textures::WhitePixelSize);
	}
};
