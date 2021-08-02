#pragma once

#include "ImGui/imgui.h"

#include "../Utils/TextureUtils.h"

#include "Fonts.h"
#include "Textures.h"

#include "AnimatedLogin.h"

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

	//animations
	static inline std::vector< IDirect3DTexture9*> LoginBackgroundTextureAnimated;

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

		std::vector<std::vector<unsigned char>> loginAnim;
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim0), std::end(AnimatedLogin::Anim0)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim1), std::end(AnimatedLogin::Anim1)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim2), std::end(AnimatedLogin::Anim2)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim3), std::end(AnimatedLogin::Anim3)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim4), std::end(AnimatedLogin::Anim4)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim5), std::end(AnimatedLogin::Anim5)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim6), std::end(AnimatedLogin::Anim6)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim7), std::end(AnimatedLogin::Anim7)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim8), std::end(AnimatedLogin::Anim8)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim9), std::end(AnimatedLogin::Anim9)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim10), std::end(AnimatedLogin::Anim10)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim11), std::end(AnimatedLogin::Anim11)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim12), std::end(AnimatedLogin::Anim12)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim13), std::end(AnimatedLogin::Anim13)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim14), std::end(AnimatedLogin::Anim14)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim15), std::end(AnimatedLogin::Anim15)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim16), std::end(AnimatedLogin::Anim16)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim17), std::end(AnimatedLogin::Anim17)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim18), std::end(AnimatedLogin::Anim18)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim19), std::end(AnimatedLogin::Anim19)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim20), std::end(AnimatedLogin::Anim20)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim21), std::end(AnimatedLogin::Anim21)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim22), std::end(AnimatedLogin::Anim22)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim23), std::end(AnimatedLogin::Anim23)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim24), std::end(AnimatedLogin::Anim24)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim25), std::end(AnimatedLogin::Anim25)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim26), std::end(AnimatedLogin::Anim26)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim27), std::end(AnimatedLogin::Anim27)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim28), std::end(AnimatedLogin::Anim28)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim29), std::end(AnimatedLogin::Anim29)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim30), std::end(AnimatedLogin::Anim30)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim31), std::end(AnimatedLogin::Anim31)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim32), std::end(AnimatedLogin::Anim32)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim33), std::end(AnimatedLogin::Anim33)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim34), std::end(AnimatedLogin::Anim34)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim35), std::end(AnimatedLogin::Anim35)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim36), std::end(AnimatedLogin::Anim36)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim37), std::end(AnimatedLogin::Anim37)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim38), std::end(AnimatedLogin::Anim38)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim39), std::end(AnimatedLogin::Anim39)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim40), std::end(AnimatedLogin::Anim40)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim41), std::end(AnimatedLogin::Anim41)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim42), std::end(AnimatedLogin::Anim42)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim43), std::end(AnimatedLogin::Anim43)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim44), std::end(AnimatedLogin::Anim44)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim45), std::end(AnimatedLogin::Anim45)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim46), std::end(AnimatedLogin::Anim46)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim47), std::end(AnimatedLogin::Anim47)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim48), std::end(AnimatedLogin::Anim48)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim49), std::end(AnimatedLogin::Anim49)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim50), std::end(AnimatedLogin::Anim50)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim51), std::end(AnimatedLogin::Anim51)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim52), std::end(AnimatedLogin::Anim52)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim53), std::end(AnimatedLogin::Anim53)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim54), std::end(AnimatedLogin::Anim54)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim55), std::end(AnimatedLogin::Anim55)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim56), std::end(AnimatedLogin::Anim56)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim57), std::end(AnimatedLogin::Anim57)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim58), std::end(AnimatedLogin::Anim58)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim59), std::end(AnimatedLogin::Anim59)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim60), std::end(AnimatedLogin::Anim60)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim61), std::end(AnimatedLogin::Anim61)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim62), std::end(AnimatedLogin::Anim62)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim63), std::end(AnimatedLogin::Anim63)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim64), std::end(AnimatedLogin::Anim64)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim65), std::end(AnimatedLogin::Anim65)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim66), std::end(AnimatedLogin::Anim66)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim67), std::end(AnimatedLogin::Anim67)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim68), std::end(AnimatedLogin::Anim68)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim69), std::end(AnimatedLogin::Anim69)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim70), std::end(AnimatedLogin::Anim70)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim71), std::end(AnimatedLogin::Anim71)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim72), std::end(AnimatedLogin::Anim72)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim73), std::end(AnimatedLogin::Anim73)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim74), std::end(AnimatedLogin::Anim74)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim75), std::end(AnimatedLogin::Anim75)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim76), std::end(AnimatedLogin::Anim76)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim77), std::end(AnimatedLogin::Anim77)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim78), std::end(AnimatedLogin::Anim78)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim79), std::end(AnimatedLogin::Anim79)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim80), std::end(AnimatedLogin::Anim80)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim81), std::end(AnimatedLogin::Anim81)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim82), std::end(AnimatedLogin::Anim82)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim83), std::end(AnimatedLogin::Anim83)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim84), std::end(AnimatedLogin::Anim84)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim85), std::end(AnimatedLogin::Anim85)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim86), std::end(AnimatedLogin::Anim86)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim87), std::end(AnimatedLogin::Anim87)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim88), std::end(AnimatedLogin::Anim88)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim89), std::end(AnimatedLogin::Anim89)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim90), std::end(AnimatedLogin::Anim90)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim91), std::end(AnimatedLogin::Anim91)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim92), std::end(AnimatedLogin::Anim92)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim93), std::end(AnimatedLogin::Anim93)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim94), std::end(AnimatedLogin::Anim94)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim95), std::end(AnimatedLogin::Anim95)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim96), std::end(AnimatedLogin::Anim96)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim97), std::end(AnimatedLogin::Anim97)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim98), std::end(AnimatedLogin::Anim98)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim99), std::end(AnimatedLogin::Anim99)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim100), std::end(AnimatedLogin::Anim100)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim101), std::end(AnimatedLogin::Anim101)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim102), std::end(AnimatedLogin::Anim102)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim103), std::end(AnimatedLogin::Anim103)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim104), std::end(AnimatedLogin::Anim104)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim105), std::end(AnimatedLogin::Anim105)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim106), std::end(AnimatedLogin::Anim106)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim107), std::end(AnimatedLogin::Anim107)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim108), std::end(AnimatedLogin::Anim108)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim109), std::end(AnimatedLogin::Anim109)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim110), std::end(AnimatedLogin::Anim110)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim111), std::end(AnimatedLogin::Anim111)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim112), std::end(AnimatedLogin::Anim112)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim113), std::end(AnimatedLogin::Anim113)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim114), std::end(AnimatedLogin::Anim114)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim115), std::end(AnimatedLogin::Anim115)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim116), std::end(AnimatedLogin::Anim116)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim117), std::end(AnimatedLogin::Anim117)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim118), std::end(AnimatedLogin::Anim118)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim119), std::end(AnimatedLogin::Anim119)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim120), std::end(AnimatedLogin::Anim120)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim121), std::end(AnimatedLogin::Anim121)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim122), std::end(AnimatedLogin::Anim122)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim123), std::end(AnimatedLogin::Anim123)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim124), std::end(AnimatedLogin::Anim124)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim125), std::end(AnimatedLogin::Anim125)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim126), std::end(AnimatedLogin::Anim126)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim127), std::end(AnimatedLogin::Anim127)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim128), std::end(AnimatedLogin::Anim128)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim129), std::end(AnimatedLogin::Anim129)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim130), std::end(AnimatedLogin::Anim130)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim131), std::end(AnimatedLogin::Anim131)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim132), std::end(AnimatedLogin::Anim132)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim133), std::end(AnimatedLogin::Anim133)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim134), std::end(AnimatedLogin::Anim134)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim135), std::end(AnimatedLogin::Anim135)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim136), std::end(AnimatedLogin::Anim136)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim137), std::end(AnimatedLogin::Anim137)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim138), std::end(AnimatedLogin::Anim138)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim139), std::end(AnimatedLogin::Anim139)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim140), std::end(AnimatedLogin::Anim140)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim141), std::end(AnimatedLogin::Anim141)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim142), std::end(AnimatedLogin::Anim142)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim143), std::end(AnimatedLogin::Anim143)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim144), std::end(AnimatedLogin::Anim144)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim145), std::end(AnimatedLogin::Anim145)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim146), std::end(AnimatedLogin::Anim146)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim147), std::end(AnimatedLogin::Anim147)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim148), std::end(AnimatedLogin::Anim148)));
		loginAnim.push_back(std::vector<unsigned char>(std::begin(AnimatedLogin::Anim149), std::end(AnimatedLogin::Anim149)));

		for (int i = 0; i < loginAnim.size(); i++)
			LoginBackgroundTextureAnimated.push_back(TextureUtils::CreateTexture(loginAnim[i].data(), loginAnim[i].size()));
	}
};
