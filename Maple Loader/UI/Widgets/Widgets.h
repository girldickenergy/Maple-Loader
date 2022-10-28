#pragma once

#include <imgui.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#include <vector>
#include <string>

class Widgets
{
	inline static auto vector_getter = [](void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<std::string>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector.at(idx).c_str();
		return true;
	};

	static bool items_ArrayGetter(void* data, int idx, const char** out_text);
	static float calcMaxPopupHeightFromItemCount(int items_count);
public:
	static bool Selectable(const char* label, bool selected = false, ImGuiSelectableFlags flags = 0, const ImVec2& size_arg = ImVec2(0, 0));
	static bool ButtonEx(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags);
	static bool Button(const char* label, const ImVec2& size_arg = ImVec2(0, 0));
	static bool BeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags = 0);
	static bool BeginComboPopup(ImGuiID popup_id, const ImRect& bb, ImGuiComboFlags flags);
	static bool Combo(const char* label, int* current_item, bool(*items_getter)(void* data, int idx, const char** out_text), void* data, int items_count, int popup_max_height_in_items = -1);
	static bool Combo(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1);
	static void LinkEx(const char* label, const char* url, ImVec4 vColor, ImVec4 vHoveredColor, ImVec4 vClickColor);
	static void Link(const char* label, const char* url, bool useSmallFont = true);
	static void LinkWithText(const char* label, const char* url, const char* text, bool useSmallFont = true);
	static bool TreeNode(const char* label, void* icon);
	static void Gradient(ImVec2 startPosition, ImVec2 endPosition, ImVec4 startColour, ImVec4 endColour);
	static bool Combo(const char* label, int* currIndex, std::vector<std::string>& values);
};
