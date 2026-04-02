//
// Created by jamie on 2025/4/15.
//

#pragma once

#include "imgui.h"

#include <vector>

struct MultiSelection : ImGuiSelectionBasicStorage
{
    constexpr auto Begin(ImGuiMultiSelectFlags flags, const int itemSize) const { return ImGui::BeginMultiSelect(flags, Size, itemSize); }

    constexpr auto ContainsIndex(int idx) -> bool { return Contains(GetStorageIdFromIndex(idx)); }
};

namespace SosGui::widgets
{
bool DrawNearActorsCombo(const std::vector<RE::Actor *> &nearActors, RE::Actor **selectedActor, RE::Actor *defaultActor);
}
