#pragma once

#include <imgui.h>

class StyleProvider
{
	static void loadFonts();
	static void loadColours();
	static void loadTextures();
public:
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

	//style variables
	static inline ImVec2 LoginWindowSize = ImVec2(500, 440);
	static inline ImVec2 LoginWindowPadding = ImVec2(50, 50);
	static inline ImVec2 LoginWindowInnerPadding = ImVec2(25, 25);
	static inline ImVec2 MainWindowSize = ImVec2(800, 600);
	static inline ImVec2 MainWindowSideBarSize = ImVec2(250, 600);
	static inline ImVec2 WindowControlSize = ImVec2(60, 40);
	static inline ImVec2 Padding = ImVec2(10, 10);

	//main colours
	static inline ImVec4 AccentColour = ImColor(232, 93, 155, 255).Value;
	static inline ImVec4 MenuColour = ImColor(65, 65, 65, 255).Value;
	static inline ImVec4 ControlColour = ImColor(76, 76, 76, 255).Value;
	static inline ImVec4 MenuColourDark;
	static inline ImVec4 MenuColourVeryDark;
	static inline ImVec4 MottoColour;
	static inline ImVec4 LinkColour;
	static inline ImVec4 LinkHoveredColour;
	static inline ImVec4 LinkActiveColour;
	static inline ImVec4 CheatBannerGradientStartColour;
	static inline ImVec4 CheatBannerGradientEndColour;

	//window controls
	static inline ImVec4 WindowControlTextColour;
	static inline ImVec4 MinimizeButtonColour;
	static inline ImVec4 MinimizeButtonHoveredColour;
	static inline ImVec4 MinimizeButtonActiveColour;
	static inline ImVec4 CloseButtonColour;
	static inline ImVec4 CloseButtonHoveredColour;
	static inline ImVec4 CloseButtonActiveColour;

	//textures/texture sizes
	static inline void* MapleLogoTexture;
	static inline void* ChevronIconTexture;
	static inline ImVec2 CheatBannerSize = ImVec2(550, 225);

	static void Initialize();
	static void Shutdown();
};