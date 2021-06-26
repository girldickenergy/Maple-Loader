#pragma once

#include "ImGui/imgui.h"

#include "Fonts.h"

class StyleProvider
{
public:
	static inline ImFont* FontTitle;
	static inline ImFont* FontDefault;

	static inline ImVec4 WindowColour = ImVec4(ImColor(65, 65, 65, 255));
	static inline ImVec4 ButtonColour = ImVec4(ImColor(71, 176, 229, 255));
	static inline ImVec4 ControlColour = ImVec4(ImColor(232, 93, 155, 255));
	static inline ImVec4 TitleColour = ImVec4(ImColor(232, 93, 155, 255));
	static inline ImVec4 ExpirationColour = ImColor(191, 191, 191);
	static inline ImVec4 TitleBarColour;
	static inline ImVec4 TextBoxColour;
	static inline ImVec4 WindowControl;
	static inline ImVec4 WindowControlHovered;
	static inline ImVec4 WindowControlActive;
	static inline ImVec4 DropDownArrowColour;
	static inline ImVec4 DropDownArrowHoveredColour;

	static inline int TitleBarHeight = 30;
	static inline ImVec2 WindowPadding = ImVec2(30, 20);

	static void LoadFonts()
	{
		ImGuiIO& io = ImGui::GetIO();

		FontTitle = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaData, Fonts::ComfortaaSize, 22);
		FontDefault = io.Fonts->AddFontFromMemoryCompressedTTF(Fonts::ComfortaaData, Fonts::ComfortaaSize, 16);
	}

	static void LoadColours()
	{
		ImGuiStyle& style = ImGui::GetStyle();

		TitleBarColour = ImVec4(WindowColour.x - (30.0f / 255.0f), WindowColour.y - (30.0f / 255.0f), WindowColour.z - (30.0f / 255.0f), 1.0f);
		WindowControl = ImVec4(TitleBarColour.x + 0.1f, TitleBarColour.y + 0.1f, TitleBarColour.z + 0.1f, TitleBarColour.w);
		WindowControlHovered = ImVec4(WindowControl.x + 0.05f, WindowControl.y + 0.05f, WindowControl.z + 0.05f, WindowControl.w);
		WindowControlActive = ImVec4(WindowControlHovered.x + 0.05f, WindowControlHovered.y + 0.05f, WindowControlHovered.z + 0.05f, WindowControlHovered.w);
		
		style.Colors[ImGuiCol_WindowBg] = WindowColour;

		style.Colors[ImGuiCol_Button] = ButtonColour;
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(ButtonColour.x + 0.05f, ButtonColour.y + 0.05f, ButtonColour.z + 0.05f, ButtonColour.w);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(ButtonColour.x + 0.1f, ButtonColour.y + 0.1f, ButtonColour.z + 0.1f, ButtonColour.w);

		style.Colors[ImGuiCol_FrameBg] = ControlColour;
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(ControlColour.x + 0.05f, ControlColour.y + 0.05f, ControlColour.z + 0.05f, ControlColour.w);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(ControlColour.x + 0.1f, ControlColour.y + 0.1f, ControlColour.z + 0.1f, ControlColour.w);

		TextBoxColour = ImVec4(WindowColour.x + 0.05f, WindowColour.y + 0.05f, WindowColour.z + 0.05f, WindowColour.w);
		style.Colors[ImGuiCol_Border] = ImVec4(WindowColour.x - 0.05f, WindowColour.y - 0.05f, WindowColour.z - 0.05f, WindowColour.w);
		
		DropDownArrowColour = ImVec4(ControlColour.x - 0.05f, ControlColour.y - 0.05f, ControlColour.z - 0.05f, ControlColour.w);
		DropDownArrowHoveredColour = ImVec4(ControlColour.x - 0.1f, ControlColour.y - 0.1f, ControlColour.z - 0.1f, ControlColour.w);
		style.Colors[ImGuiCol_Header] = ControlColour;
		style.Colors[ImGuiCol_HeaderActive] = ControlColour;
		style.Colors[ImGuiCol_HeaderHovered] = ControlColour;
	}
};
