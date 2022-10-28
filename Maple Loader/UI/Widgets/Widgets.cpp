#include "Widgets.h"

#include <windows.h>
#include <shellapi.h>
#include <map>

#include "../StyleProvider.h"

bool Widgets::items_ArrayGetter(void* data, int idx, const char** out_text)
{
    const char* const* items = (const char* const*)data;
    if (out_text)
        *out_text = items[idx];
    return true;
}

float Widgets::calcMaxPopupHeightFromItemCount(int items_count)
{
    ImGuiContext& g = *GImGui;
    if (items_count <= 0)
        return FLT_MAX;
    return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
}

bool Widgets::Selectable(const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2& size_arg)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    // Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit a larger/spanning rectangle.
    ImGuiID id = window->GetID(label);
    ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    ImVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
    ImVec2 pos = window->DC.CursorPos;
    pos.y += window->DC.CurrLineTextBaseOffset;
    ImGui::ItemSize(size, 0.0f);

    // Fill horizontal space
    // We don't support (size < 0.0f) in Selectable() because the ItemSpacing extension would make explicitly right-aligned sizes not visibly match other widgets.
    const bool span_all_columns = (flags & ImGuiSelectableFlags_SpanAllColumns) != 0;
    const float min_x = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
    const float max_x = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
    if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_SpanAvailWidth))
        size.x = ImMax(label_size.x, max_x - min_x);

    // Text stays at the submission position, but bounding box may be extended on both sides
    const ImVec2 text_min = ImVec2(pos.x + style.FramePadding.x, pos.y);
    const ImVec2 text_max(min_x + size.x + style.FramePadding.x, pos.y + size.y);

    // Selectables are meant to be tightly packed together with no click-gap, so we extend their box to cover spacing between selectable.
    ImRect bb(min_x, pos.y, text_max.x - style.FramePadding.x, text_max.y);
    if ((flags & ImGuiSelectableFlags_NoPadWithHalfSpacing) == 0)
    {
        const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
        const float spacing_y = style.ItemSpacing.y;
        const float spacing_L = IM_FLOOR(spacing_x * 0.50f);
        const float spacing_U = IM_FLOOR(spacing_y * 0.50f);
        bb.Min.x -= spacing_L;
        bb.Min.y -= spacing_U;
        bb.Max.x += (spacing_x - spacing_L);
        bb.Max.y += (spacing_y - spacing_U);
    }
    //if (g.IO.KeyCtrl) { GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(0, 255, 0, 255)); }

    // Modify ClipRect for the ItemAdd(), faster than doing a PushColumnsBackground/PushTableBackground for every Selectable..
    const float backup_clip_rect_min_x = window->ClipRect.Min.x;
    const float backup_clip_rect_max_x = window->ClipRect.Max.x;
    if (span_all_columns)
    {
        window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
        window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
    }

    const bool disabled_item = (flags & ImGuiSelectableFlags_Disabled) != 0;
    const bool item_add = ImGui::ItemAdd(bb, id, NULL, disabled_item ? ImGuiItemFlags_Disabled : ImGuiItemFlags_None);
    if (span_all_columns)
    {
        window->ClipRect.Min.x = backup_clip_rect_min_x;
        window->ClipRect.Max.x = backup_clip_rect_max_x;
    }

    if (!item_add)
        return false;

    if (span_all_columns)
    {
        window->ClipRect.Min.x = backup_clip_rect_min_x;
        window->ClipRect.Max.x = backup_clip_rect_max_x;
    }

    if (!item_add)
        return false;

    // FIXME: We can standardize the behavior of those two, we could also keep the fast path of override ClipRect + full push on render only,
    // which would be advantageous since most selectable are not selected.
    if (span_all_columns && window->DC.CurrentColumns)
        ImGui::PushColumnsBackground();
    else if (span_all_columns && g.CurrentTable)
        ImGui::TablePushBackgroundChannel();

    // We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
    ImGuiButtonFlags button_flags = 0;
    if (flags & ImGuiSelectableFlags_NoHoldingActiveID) { button_flags |= ImGuiButtonFlags_NoHoldingActiveId; }
    if (flags & ImGuiSelectableFlags_SelectOnClick) { button_flags |= ImGuiButtonFlags_PressedOnClick; }
    if (flags & ImGuiSelectableFlags_SelectOnRelease) { button_flags |= ImGuiButtonFlags_PressedOnRelease; }
    if (flags & ImGuiSelectableFlags_AllowDoubleClick) { button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick; }
    if (flags & ImGuiSelectableFlags_AllowItemOverlap) { button_flags |= ImGuiButtonFlags_AllowItemOverlap; }

    const bool was_selected = selected;
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, button_flags);

    // Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with gamepad/keyboard
    if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover)))
    {
        if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
        {
            ImGui::SetNavID(id, window->DC.NavLayerCurrent, window->DC.NavFocusScopeIdCurrent, ImRect(bb.Min - window->Pos, bb.Max - window->Pos));
            g.NavDisableHighlight = true;
        }
    }
    if (pressed)
        ImGui::MarkItemEdited(id);

    if (flags & ImGuiSelectableFlags_AllowItemOverlap)
        ImGui::SetItemAllowOverlap();

    // In this branch, Selectable() cannot toggle the selection so this will never trigger.
    if (selected != was_selected) //-V547
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

    const float elapsed = ImGui::GetIO().DeltaTime * 1000.f;
    const float animationTime = 150.f;

    static std::map<ImGuiID, float> hoverAnimationMap;
    auto hoverAnimation = hoverAnimationMap.find(id);
    if (hoverAnimation == hoverAnimationMap.end())
    {
        hoverAnimationMap.insert({ id, 0.f });
        hoverAnimation = hoverAnimationMap.find(id);
    }

    if (hovered && hoverAnimation->second < 1.f)
        hoverAnimation->second += elapsed / animationTime;

    if (!hovered && hoverAnimation->second > 0.f)
        hoverAnimation->second -= elapsed / animationTime;

    hoverAnimation->second = ImClamp(hoverAnimation->second, 0.f, 1.f);

    static std::map<ImGuiID, float> holdAnimationMap;
    auto holdAnimation = holdAnimationMap.find(id);
    if (holdAnimation == holdAnimationMap.end())
    {
        holdAnimationMap.insert({ id, 0.f });
        holdAnimation = holdAnimationMap.find(id);
    }

    if (held && holdAnimation->second < 1.f)
        holdAnimation->second += elapsed / animationTime;

    if (!held && holdAnimation->second > 0.f)
        holdAnimation->second -= elapsed / animationTime;

    holdAnimation->second = ImClamp(holdAnimation->second, 0.f, 1.f);

    // Render
    if (held && (flags & ImGuiSelectableFlags_DrawHoveredWhenHeld))
        hovered = true;

    ImColor col = ImLerp(selected ? style.Colors[ImGuiCol_Header] : ImVec4(style.Colors[ImGuiCol_Header].x, style.Colors[ImGuiCol_Header].y, style.Colors[ImGuiCol_Header].z, 0), style.Colors[ImGuiCol_HeaderHovered], hoverAnimation->second);
    if (held)
        col = ImLerp(col, style.Colors[ImGuiCol_HeaderActive], holdAnimation->second);
    else
        col = ImLerp(style.Colors[ImGuiCol_HeaderActive], col, 1.f - holdAnimation->second);

    ImGui::RenderFrame(bb.Min, bb.Max, col, false, style.FrameRounding);
    ImGui::RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);

    if (span_all_columns && window->DC.CurrentColumns)
        ImGui::PopColumnsBackground();
    else if (span_all_columns && g.CurrentTable)
        ImGui::TablePopBackgroundChannel();

    if (flags & ImGuiSelectableFlags_Disabled) ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
    ImGui::RenderTextClipped(text_min, text_max, label, NULL, &label_size, style.SelectableTextAlign, &bb);
    if (flags & ImGuiSelectableFlags_Disabled) ImGui::PopStyleColor();

    // Automatically close popups
    if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && !(g.LastItemData.InFlags & ImGuiItemFlags_SelectableDontClosePopup))
        ImGui::CloseCurrentPopup();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);
    return pressed;
}


bool Widgets::ButtonEx(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_Repeat;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

    const float elapsed = ImGui::GetIO().DeltaTime * 1000.f;
    const float animationTime = 150.f;

    static std::map<ImGuiID, float> hoverAnimationMap;
    auto hoverAnimation = hoverAnimationMap.find(id);
    if (hoverAnimation == hoverAnimationMap.end())
    {
        hoverAnimationMap.insert({ id, 0.f });
        hoverAnimation = hoverAnimationMap.find(id);
    }

    if (hovered && hoverAnimation->second < 1.f)
        hoverAnimation->second += elapsed / animationTime;

    if (!hovered && hoverAnimation->second > 0.f)
        hoverAnimation->second -= elapsed / animationTime;

    hoverAnimation->second = ImClamp(hoverAnimation->second, 0.f, 1.f);

    static std::map<ImGuiID, float> holdAnimationMap;
    auto holdAnimation = holdAnimationMap.find(id);
    if (holdAnimation == holdAnimationMap.end())
    {
        holdAnimationMap.insert({ id, 0.f });
        holdAnimation = holdAnimationMap.find(id);
    }

    if (held && holdAnimation->second < 1.f)
        holdAnimation->second += elapsed / animationTime;

    if (!held && holdAnimation->second > 0.f)
        holdAnimation->second -= elapsed / animationTime;

    holdAnimation->second = ImClamp(holdAnimation->second, 0.f, 1.f);

    // Render
    ImGui::RenderNavHighlight(bb, id);

    ImColor col = ImLerp(style.Colors[ImGuiCol_Button], style.Colors[ImGuiCol_ButtonHovered], hoverAnimation->second);
    if (held)
        col = ImLerp(col, style.Colors[ImGuiCol_ButtonActive], holdAnimation->second);
    else
        col = ImLerp(style.Colors[ImGuiCol_ButtonActive], col, 1.f - holdAnimation->second);

    ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

    if (g.LogEnabled)
        ImGui::LogSetNextTextDecoration("[", "]");
    ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

    // Automatically close popups
    //if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
    //    CloseCurrentPopup();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}

bool Widgets::Button(const char* label, const ImVec2& size_arg)
{
    return ButtonEx(label, size_arg, ImGuiButtonFlags_None);
}

bool Widgets::BeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    ImGuiNextWindowDataFlags backup_next_window_data_flags = g.NextWindowData.Flags;
    g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
    if (window->SkipItems)
        return false;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    IM_ASSERT((flags & (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)) != (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)); // Can't use both flags together

    const ImVec2 arrow_size = (flags & ImGuiComboFlags_NoArrowButton) ? ImVec2(0, 0) : ImVec2(ImGui::GetFrameHeight() / 1.5, ImGui::GetFrameHeight() / 1.5 * 0.625f);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    const float expected_w = ImGui::CalcItemWidth();
    const float w = (flags & ImGuiComboFlags_NoPreview) ? arrow_size.x : expected_w;
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id, &frame_bb))
        return false;

    // Open on click
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(frame_bb, id, &hovered, &held);
    const ImGuiID popup_id = ImHashStr("##ComboPopup", 0, id);
    bool popup_open = ImGui::IsPopupOpen(popup_id, ImGuiPopupFlags_None);
    if (pressed && !popup_open)
    {
        ImGui::OpenPopupEx(popup_id, ImGuiPopupFlags_None);
        popup_open = true;
    }

    const float elapsed = ImGui::GetIO().DeltaTime * 1000.f;
    const float animationTime = 150.f;

    static std::map<ImGuiID, float> hoverAnimationMap;
    auto hoverAnimation = hoverAnimationMap.find(id);
    if (hoverAnimation == hoverAnimationMap.end())
    {
        hoverAnimationMap.insert({ id, 0.f });
        hoverAnimation = hoverAnimationMap.find(id);
    }

    if (hovered && hoverAnimation->second < 1.f)
        hoverAnimation->second += elapsed / animationTime;

    if (!hovered && hoverAnimation->second > 0.f)
        hoverAnimation->second -= elapsed / animationTime;

    hoverAnimation->second = ImClamp(hoverAnimation->second, 0.f, 1.f);

    // Render shape
    const ImColor frame_col = ImLerp(style.Colors[ImGuiCol_FrameBg], style.Colors[ImGuiCol_FrameBgHovered], hoverAnimation->second);
    const float value_x2 = ImMax(frame_bb.Min.x, frame_bb.Max.x - arrow_size.x);
    ImGui::RenderNavHighlight(frame_bb, id);
    if (!(flags & ImGuiComboFlags_NoPreview))
        window->DrawList->AddRectFilled(frame_bb.Min, frame_bb.Max, frame_col, style.FrameRounding, ImDrawFlags_RoundCornersAll);
    if (!(flags & ImGuiComboFlags_NoArrowButton))
    {
        window->DrawList->AddImage(StyleProvider::ChevronIconTexture, ImVec2(frame_bb.Max.x - arrow_size.x - (frame_bb.Max.y - frame_bb.Min.y - arrow_size.y) / 2, frame_bb.Min.y + (frame_bb.Max.y - frame_bb.Min.y) / 2 - arrow_size.y / 2), ImVec2(frame_bb.Max.x - (frame_bb.Max.y - frame_bb.Min.y - arrow_size.y) / 2, frame_bb.Min.y + (frame_bb.Max.y - frame_bb.Min.y) / 2 + arrow_size.y / 2), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImGuiCol_Text));
    }
    ImGui::RenderFrameBorder(frame_bb.Min, frame_bb.Max, style.FrameRounding);

    // Custom preview
    if (flags & ImGuiComboFlags_CustomPreview)
    {
        g.ComboPreviewData.PreviewRect = ImRect(frame_bb.Min.x, frame_bb.Min.y, value_x2, frame_bb.Max.y);
        IM_ASSERT(preview_value == NULL || preview_value[0] == 0);
        preview_value = NULL;
    }

    // Render preview and label
    if (preview_value != NULL && !(flags & ImGuiComboFlags_NoPreview))
    {
        ImVec2 preview_pos = frame_bb.Min + style.FramePadding;
        if (g.LogEnabled)
            ImGui::LogSetNextTextDecoration("{", "}");
        ImGui::RenderTextClipped(preview_pos, ImVec2(value_x2, frame_bb.Max.y), preview_value, NULL, NULL, ImVec2(0.0f, 0.0f));
    }
    if (label_size.x > 0)
        ImGui::RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

    if (!popup_open)
        return false;

    g.NextWindowData.Flags = backup_next_window_data_flags;
    return BeginComboPopup(popup_id, frame_bb, flags);
}

bool Widgets::BeginComboPopup(ImGuiID popup_id, const ImRect& bb, ImGuiComboFlags flags)
{
    ImGuiContext& g = *GImGui;
    if (!ImGui::IsPopupOpen(popup_id, ImGuiPopupFlags_None))
    {
        g.NextWindowData.ClearFlags();
        return false;
    }

    // Set popup size
    float w = bb.GetWidth();
    if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint)
    {
        g.NextWindowData.SizeConstraintRect.Min.x = ImMax(g.NextWindowData.SizeConstraintRect.Min.x, w);
    }
    else
    {
        if ((flags & ImGuiComboFlags_HeightMask_) == 0)
            flags |= ImGuiComboFlags_HeightRegular;
        IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiComboFlags_HeightMask_)); // Only one
        int popup_max_height_in_items = -1;
        if (flags & ImGuiComboFlags_HeightRegular)     popup_max_height_in_items = 8;
        else if (flags & ImGuiComboFlags_HeightSmall)  popup_max_height_in_items = 4;
        else if (flags & ImGuiComboFlags_HeightLarge)  popup_max_height_in_items = 20;
        ImGui::SetNextWindowSizeConstraints(ImVec2(w, 0.0f), ImVec2(FLT_MAX, calcMaxPopupHeightFromItemCount(popup_max_height_in_items)));
    }

    // This is essentially a specialized version of BeginPopupEx()
    char name[16];
    ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g.BeginPopupStack.Size); // Recycle windows based on depth

    // Set position given a custom constraint (peak into expected window size so we can position it)
    // FIXME: This might be easier to express with an hypothetical SetNextWindowPosConstraints() function?
    // FIXME: This might be moved to Begin() or at least around the same spot where Tooltips and other Popups are calling FindBestWindowPosForPopupEx()?
    if (ImGuiWindow* popup_window = ImGui::FindWindowByName(name))
        if (popup_window->WasActive)
        {
            // Always override 'AutoPosLastDirection' to not leave a chance for a past value to affect us.
            ImVec2 size_expected = ImGui::CalcWindowNextAutoFitSize(popup_window);
            popup_window->AutoPosLastDirection = (flags & ImGuiComboFlags_PopupAlignLeft) ? ImGuiDir_Left : ImGuiDir_Down; // Left = "Below, Toward Left", Down = "Below, Toward Right (default)"
            ImRect r_outer = ImGui::GetPopupAllowedExtentRect(popup_window);
            ImVec2 pos = ImGui::FindBestWindowPosForPopupEx(bb.GetBL(), size_expected, &popup_window->AutoPosLastDirection, r_outer, bb, ImGuiPopupPositionPolicy_ComboBox);
            ImGui::SetNextWindowPos(pos + ImVec2(0, g.Style.ItemInnerSpacing.y));
        }

    // We don't use BeginPopupEx() solely because we have a custom name string, which we could make an argument to BeginPopupEx()
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;

    // Horizontally align ourselves with the framed text
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, StyleProvider::Padding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(g.Style.FramePadding.x, g.Style.WindowPadding.y));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGui::GetColorU32(ImGuiCol_FrameBgActive));
    bool ret = ImGui::Begin(name, NULL, window_flags);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    if (!ret)
    {
        ImGui::EndPopup();
        IM_ASSERT(0);   // This should never happen as we tested for IsPopupOpen() above
        return false;
    }
    return true;
}

bool Widgets::Combo(const char* label, int* current_item, bool(*items_getter)(void* data, int idx, const char** out_text), void* data, int items_count, int popup_max_height_in_items)
{
    ImGuiContext& g = *GImGui;

    // Call the getter to obtain the preview string which is a parameter to BeginCombo()
    const char* preview_value = NULL;
    if (*current_item >= 0 && *current_item < items_count)
        items_getter(data, *current_item, &preview_value);

    // The old Combo() API exposed "popup_max_height_in_items". The new more general BeginCombo() API doesn't have/need it, but we emulate it here.
    if (popup_max_height_in_items != -1 && !(g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint))
        ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, calcMaxPopupHeightFromItemCount(popup_max_height_in_items)));

    if (!BeginCombo(label, preview_value, ImGuiComboFlags_None))
        return false;

    // Display items
    // FIXME-OPT: Use clipper (but we need to disable it on the appearing frame to make sure our call to SetItemDefaultFocus() is processed)
    bool value_changed = false;
    for (int i = 0; i < items_count; i++)
    {
        ImGui::PushID((void*)(intptr_t)i);
        const bool item_selected = (i == *current_item);
        const char* item_text;
        if (!items_getter(data, i, &item_text))
            item_text = "*Unknown item*";
        if (Selectable(item_text, item_selected, ImGuiSelectableFlags_SpanAllColumns))
        {
            value_changed = true;
            *current_item = i;
        }
        if (item_selected)
            ImGui::SetItemDefaultFocus();
        ImGui::PopID();
    }

    ImGui::EndCombo();
    if (value_changed)
        ImGui::MarkItemEdited(g.LastItemData.ID);

    return value_changed;
}

bool Widgets::Combo(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items)
{
    const bool value_changed = Combo(label, current_item, items_ArrayGetter, (void*)items, items_count, popup_max_height_in_items);
    return value_changed;
}

void Widgets::LinkEx(const char* label, const char* url, ImVec4 vColor, ImVec4 vHoveredColor, ImVec4 vClickColor)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, pos + label_size);
    ImGui::ItemSize(bb, 0.0f);
    if (!ImGui::ItemAdd(bb, id))
        return;

    ImGuiButtonFlags flags = 0;
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);
    if (held || (g.HoveredId == id && g.HoveredIdPreviousFrame == id))
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    ImGui::RenderNavHighlight(bb, id);

    const float elapsed = ImGui::GetIO().DeltaTime * 1000.f;
    const float animationTime = 150.f;

    static std::map<ImGuiID, float> hoverAnimationMap;
    auto hoverAnimation = hoverAnimationMap.find(id);
    if (hoverAnimation == hoverAnimationMap.end())
    {
        hoverAnimationMap.insert({ id, 0.f });
        hoverAnimation = hoverAnimationMap.find(id);
    }

    if (hovered && hoverAnimation->second < 1.f)
        hoverAnimation->second += elapsed / animationTime;

    if (!hovered && hoverAnimation->second > 0.f)
        hoverAnimation->second -= elapsed / animationTime;

    hoverAnimation->second = ImClamp(hoverAnimation->second, 0.f, 1.f);

    static std::map<ImGuiID, float> holdAnimationMap;
    auto holdAnimation = holdAnimationMap.find(id);
    if (holdAnimation == holdAnimationMap.end())
    {
        holdAnimationMap.insert({ id, 0.f });
        holdAnimation = holdAnimationMap.find(id);
    }

    if (held && holdAnimation->second < 1.f)
        holdAnimation->second += elapsed / animationTime;

    if (!held && holdAnimation->second > 0.f)
        holdAnimation->second -= elapsed / animationTime;

    holdAnimation->second = ImClamp(holdAnimation->second, 0.f, 1.f);

    ImVec4 col = ImLerp(vColor, vHoveredColor, hoverAnimation->second);
    if (held)
        col = ImLerp(col, vClickColor, holdAnimation->second);
    else
        col = ImLerp(vClickColor, col, 1.f - holdAnimation->second);

    ImVec2 p0 = bb.Min;
    ImVec2 p1 = bb.Max;
    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImGui::RenderTextClipped(p0, p1, label, NULL, &label_size, style.ButtonTextAlign, &bb);
    ImGui::PopStyleColor(1);

    if (pressed)
    {
        ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
    }
}

void Widgets::Link(const char* label, const char* url, bool useSmallFont)
{
    if (useSmallFont)
        ImGui::PushFont(StyleProvider::FontSmall);

    LinkEx(label, url, StyleProvider::LinkColour, StyleProvider::LinkHoveredColour, StyleProvider::LinkActiveColour);

    if (useSmallFont)
        ImGui::PopFont();
}

void Widgets::LinkWithText(const char* label, const char* url, const char* text, bool useSmallFont)
{
    if (useSmallFont)
        ImGui::PushFont(StyleProvider::FontSmall);

    ImGui::Text(text);

    ImGui::SameLine(0.f, ImGui::CalcTextSize(" ").x);

    LinkEx(label, url, StyleProvider::LinkColour, StyleProvider::LinkHoveredColour, StyleProvider::LinkActiveColour);

    if (useSmallFont)
        ImGui::PopFont();
}

bool Widgets::TreeNode(const char* label, void* icon)
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

    const float elapsed = ImGui::GetIO().DeltaTime * 1000.f;
    const float animationTime = 150.f;

    static std::map<ImGuiID, float> hoverAnimationMap;
    auto hoverAnimation = hoverAnimationMap.find(id);
    if (hoverAnimation == hoverAnimationMap.end())
    {
        hoverAnimationMap.insert({ id, 0.f });
        hoverAnimation = hoverAnimationMap.find(id);
    }

    if (hovered && hoverAnimation->second < 1.f)
        hoverAnimation->second += elapsed / animationTime;

    if (!hovered && hoverAnimation->second > 0.f)
        hoverAnimation->second -= elapsed / animationTime;

    hoverAnimation->second = ImClamp(hoverAnimation->second, 0.f, 1.f);

    static std::map<ImGuiID, float> holdAnimationMap;
    auto holdAnimation = holdAnimationMap.find(id);
    if (holdAnimation == holdAnimationMap.end())
    {
        holdAnimationMap.insert({ id, 0.f });
        holdAnimation = holdAnimationMap.find(id);
    }

    if (held && holdAnimation->second < 1.f)
        holdAnimation->second += elapsed / animationTime;

    if (!held && holdAnimation->second > 0.f)
        holdAnimation->second -= elapsed / animationTime;

    holdAnimation->second = ImClamp(holdAnimation->second, 0.f, 1.f);

    ImColor col = ImLerp(ImVec4(g.Style.Colors[ImGuiCol_HeaderHovered].x, g.Style.Colors[ImGuiCol_HeaderHovered].y, g.Style.Colors[ImGuiCol_HeaderHovered].z, 0.f), g.Style.Colors[ImGuiCol_HeaderHovered], hoverAnimation->second);
    if (held)
        col = ImLerp(col, g.Style.Colors[ImGuiCol_HeaderActive], holdAnimation->second);
    else
        col = ImLerp(g.Style.Colors[ImGuiCol_HeaderActive], col, 1.f - holdAnimation->second);

    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImColor(col), g.Style.FrameRounding);

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

void Widgets::Gradient(ImVec2 startPosition, ImVec2 endPosition, ImVec4 startColour, ImVec4 endColour)
{
    const ImColor imStartColour = ImColor(startColour);
    const ImColor imEndColour = ImColor(endColour);

    const ImVec2 windowPos = ImGui::GetCurrentWindow()->Pos;

    ImGui::GetWindowDrawList()->AddRectFilledMultiColor(windowPos + startPosition, windowPos + endPosition, imStartColour, imStartColour, imEndColour, imEndColour);
}

bool Widgets::Combo(const char* label, int* currIndex, std::vector<std::string>& values)
{
    if (values.empty())
        return false;

    return Combo(label, currIndex, vector_getter, static_cast<void*>(&values), values.size());
}
