#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include "StyleProvider.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"

class Widgets
{
	inline static auto vector_getter = [](void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<std::string>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector.at(idx).c_str();
		return true;
	};
public:
	static void LinkEx(const char* label, const char* url, ImVec4 vColor, ImVec4 vHoveredColor, ImVec4 vClickColor)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;
		
		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
		ImVec2 pos = window->DC.CursorPos;
		ImVec2 size = ImGui::CalcItemSize(ImVec2(0.0f, 0.0f), label_size.x + style.FramePadding.x * 1.0f, label_size.y);
		const ImRect bb(pos, pos + size);
		ImGui::ItemSize(bb, 0.0f);
		if (!ImGui::ItemAdd(bb, id))
			return;
		
		ImGuiButtonFlags flags = 0;
		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);
		if (held || (g.HoveredId == id && g.HoveredIdPreviousFrame == id))
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		ImGui::RenderNavHighlight(bb, id);
		ImVec4 col = (hovered && held) ? vClickColor : hovered ? vHoveredColor : vColor;
		ImVec2 p0 = bb.Min;
		ImVec2 p1 = bb.Max;
		if (hovered && held)
		{
			p0 += ImVec2(1, 1);
			p1 += ImVec2(1, 1);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, col);
		ImGui::RenderTextClipped(p0, p1, label, NULL, &label_size, style.ButtonTextAlign, &bb);
		ImGui::PopStyleColor(1);

		if (pressed)
		{
			ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
		}
	}

	static void Link(const char* label, const char* url, bool forceFont = true)
	{
		if (forceFont)
			ImGui::PushFont(StyleProvider::FontSmall);
		
		LinkEx(label, url, StyleProvider::LinkColour, StyleProvider::LinkHoveredColour, StyleProvider::LinkActiveColour);

		if (forceFont)
			ImGui::PopFont();
	}

	static void LinkWithText(const char* label, const char* url, const char* text, ImVec4 textColour = ImVec4(1, 1, 1, 1), bool forceFont = true)
	{
		if (forceFont)
			ImGui::PushFont(StyleProvider::FontSmall);
		
		ImGui::TextColored(textColour, text);
		ImGui::SameLine();
		LinkEx(label, url, StyleProvider::LinkColour, StyleProvider::LinkHoveredColour, StyleProvider::LinkActiveColour);
		
		if (forceFont)
			ImGui::PopFont();
	}

	static bool TextBox(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
		bool result = ImGui::InputText(label, buf, buf_size, flags, callback, user_data);
		ImGui::PopStyleVar();

		return result;
	}

	static void Gradient(ImVec2 startPosition, ImVec2 endPosition, ImVec4 startColour, ImVec4 endColour)
	{
		ImColor imStartColour = ImColor(startColour);
		ImColor imEndColour = ImColor(endColour);

		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(startPosition, endPosition, imStartColour, imStartColour, imEndColour, imEndColour);
	}

	static bool TreeNode(const char* label, IDirect3DTexture9* icon)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;

		ImGuiID id = window->GetID(label);
		ImVec2 pos = window->DC.CursorPos;
		ImRect bb(pos, ImVec2(pos.x + ImGui::GetContentRegionAvail().x, pos.y + g.FontSize + g.Style.FramePadding.y * 2));
		bool opened = ImGui::TreeNodeBehaviorIsOpen(id);
		bool hovered, held;
		if (ImGui::ButtonBehavior(bb, id, &hovered, &held, true))
			window->DC.StateStorage->SetInt(id, opened ? 0 : 1);
		if (hovered || held)
			window->DrawList->AddRectFilled(bb.Min, bb.Max, ImColor(ImGui::GetStyle().Colors[(held ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered)]), 5);

		float button_sz = g.FontSize;
		float textOffset = g.FontSize + g.Style.FramePadding.x * 2;
		window->DrawList->AddImage(icon, ImVec2(pos.x + textOffset, pos.y + g.Style.FramePadding.y), ImVec2(pos.x + button_sz + textOffset, pos.y + button_sz + g.Style.FramePadding.y));
		ImGui::RenderArrow(window->DrawList, pos + g.Style.FramePadding / 0.75f, ImColor(255, 255, 255, 255), opened ? ImGuiDir_Down : ImGuiDir_Right, 0.75f);
		ImGui::RenderText(ImVec2(pos.x + textOffset + button_sz + g.Style.ItemInnerSpacing.x, pos.y + g.Style.FramePadding.y), label);

		ImGui::ItemSize(bb, g.Style.FramePadding.y);
		ImGui::ItemAdd(bb, id);

		if (opened)
			ImGui::TreePush(label);
		
		return opened;
	}

	static bool Combo(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return ImGui::Combo(label, currIndex, vector_getter,
			static_cast<void*>(&values), values.size());
	}

	static bool Checkbox(const char* label, bool* v)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

		const float square_sz = ImGui::GetFrameHeight();
		const ImVec2 pos = window->DC.CursorPos;
		const ImRect total_bb(pos, pos + ImVec2(square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y * 2.0f));
		ImGui::ItemSize(total_bb, style.FramePadding.y);
		if (!ImGui::ItemAdd(total_bb, id))
		{
			IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
			return false;
		}

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
		if (pressed)
		{
			*v = !(*v);
			ImGui::MarkItemEdited(id);
		}

		const ImRect check_bb(pos, pos + ImVec2(square_sz, square_sz));
		ImGui::RenderNavHighlight(total_bb, id);
		ImGui::RenderFrame(check_bb.Min, check_bb.Max, ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), true, style.FrameRounding);
		ImU32 check_col = ImGui::GetColorU32(ImGuiCol_CheckMark);
		bool mixed_value = (g.CurrentItemFlags & ImGuiItemFlags_MixedValue) != 0;
		if (mixed_value)
		{
			// Undocumented tristate/mixed/indeterminate checkbox (#2644)
			// This may seem awkwardly designed because the aim is to make ImGuiItemFlags_MixedValue supported by all widgets (not just checkbox)
			ImVec2 pad(ImMax(1.0f, IM_FLOOR(square_sz / 3.6f)), ImMax(1.0f, IM_FLOOR(square_sz / 3.6f)));
			window->DrawList->AddRectFilled(check_bb.Min + pad, check_bb.Max - pad, check_col, style.FrameRounding);
		}
		else if (*v)
		{
			const float pad = ImMax(1.0f, IM_FLOOR(square_sz / 6.0f));
			ImGui::RenderCheckMark(window->DrawList, check_bb.Min + ImVec2(pad, pad), check_col, square_sz - pad * 2.0f);
		}

		ImVec2 label_pos = ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y);
		if (g.LogEnabled)
			ImGui::LogRenderedText(&label_pos, mixed_value ? "[~]" : *v ? "[x]" : "[ ]");
		if (label_size.x > 0.0f)
			ImGui::RenderText(label_pos, label);

		IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
		return pressed;
	}
};
