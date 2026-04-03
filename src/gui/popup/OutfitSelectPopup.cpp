//
// Created by jamie on 2025/5/14.
//
#include "OutfitSelectPopup.h"

#include "data/SosUiOutfit.h"
#include "gui/icon.h"

namespace SosGui
{

bool OutfitSelectPopup::Draw(const char *nameKey, const std::vector<SosUiOutfit> &outfits, OutfitId &selectId)
{
    selectId = INVALID_OUTFIT_ID;
    if (!ImGui::BeginPopup(nameKey))
    {
        return false;
    }
    ImGui::BeginChild("##ChildRegion", ImVec2(0, 250), ImGuiChildFlags_AutoResizeX);

    ImGui::AlignTextToFramePadding();
    ImGuiUtil::IconButton(ICON_SEARCH);
    ImGui::SameLine(0.F, 0.F);
    debounce_input_.Draw("##filter", "filter outfit");

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(outfits.size()));
    while (clipper.Step())
    {
        for (size_t index = static_cast<size_t>(clipper.DisplayStart); index < static_cast<size_t>(clipper.DisplayEnd); ++index)
        {
            const auto &outfit = outfits[index];
            ImGui::PushID(static_cast<int>(index));
            if (ImGui::Selectable(outfit.GetName().c_str(), false))
            {
                selectId = outfit.GetId();
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopID();
        }
    }
    ImGui::EndChild();
    ImGui::EndPopup();
    return true;
}
} // namespace SosGui
