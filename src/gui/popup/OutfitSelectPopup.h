//
// Created by jamie on 2025/5/14.
//
#pragma once

#include "Popup.h"
#include "util/ImGuiUtil.h"

#include <vector>

namespace SosGui
{
class SosUiOutfit;

class OutfitSelectPopup : Popup::BasicPopup
{
    ImGuiUtil::DebounceInput debounce_input_;

public:
    bool Draw(const char *nameKey, const std::vector<SosUiOutfit> &outfits, OutfitId &selectId);
};
} // namespace SosGui
