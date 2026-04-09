#pragma once

#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "gui/BaseGui.h"
#include "popup/Popup.h"
#include "service/OutfitService.h"
#include "util/ImGuiUtil.h"

#include <array>
#include <functional>
#include <string>

namespace SosGui
{

class OutfitListTable final
{
    static constexpr int        MAX_OUTFIT_NAME_BYTES = 256;
    static inline EditingOutfit UNTITLED_OUTFIT;
    using DrawOutfitEntry  = std::function<void(const SosUiOutfit &, ImGuiID)>;
    using OutfitNameBuffer = std::array<char, MAX_OUTFIT_NAME_BYTES>;

    ImGuiTextFilter            name_filterer_;
    OutfitId                   editing_id_ = INVALID_OUTFIT_ID;
    EditingOutfit              editing_    = UNTITLED_OUTFIT;
    ImGuiSelectionBasicStorage multi_selection_;
    OutfitNameBuffer           outfit_name_buffer_{};
    ImGuiID                    active_input_id_          = 0;
    bool                       show_conflict_name_error_ = false;
    bool                       show_favorites_           = false;
    bool                       name_sort_ascend_         = true;

public:
    void OnRefresh();

    void Draw(const std::vector<SosUiOutfit> &outfits, OutfitService &outfitService);

    auto GetEditingOutfit() -> EditingOutfit & { return editing_; }

private:
    // refresh, filterer, favorite checkbox...
    void DrawToolWidgets(const std::vector<SosUiOutfit> &outfits, OutfitService &outfitService);
    void DrawOutfitTableContent(const std::vector<SosUiOutfit> &outfits, OutfitService &outfitService);
    void DrawCreateOutfitPopup(const std::vector<SosUiOutfit> &outfits, const char *name, OutfitService &outfitService);

    auto pass_filter(const SosUiOutfit &outfit) -> bool;

    // check MultiSelection and set all selected outfit to favorite
    void OnAcceptSetFavoriteOutfits(bool toFavorite, const std::vector<SosUiOutfit> &outfits, OutfitService &outfitService);
    void DeleteAllSelectOutfits(const std::vector<SosUiOutfit> &outfits, OutfitService &outfitService);
};
} // namespace SosGui
