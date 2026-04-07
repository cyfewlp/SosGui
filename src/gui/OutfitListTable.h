#pragma once

#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "gui/BaseGui.h"
#include "gui/widgets.h"
#include "popup/Popup.h"
#include "service/OutfitService.h"
#include "util/ImGuiUtil.h"

#include <array>
#include <functional>
#include <string>

namespace SosGui
{
class OutfitEditPanel;
class OutfitContainer;

class OutfitListTable final
{
    static constexpr int        MAX_OUTFIT_NAME_BYTES = 256;
    static inline auto          OUTFIT                = SosUiOutfit();
    static inline EditingOutfit UNTITLED_OUTFIT{OUTFIT};
    using DrawOutfitEntry  = std::function<void(const SosUiOutfit &, ImGuiID)>;
    using OutfitNameBuffer = std::array<char, MAX_OUTFIT_NAME_BYTES>;

    SosUiData       &m_uiData;
    OutfitService   &m_outfitService;
    EditingOutfit    editing_ = UNTITLED_OUTFIT;
    MultiSelection   multi_selection_;
    OutfitNameBuffer outfit_name_buffer_{};
    ImGuiID          active_input_id_ = 0;
    bool             show_favorites_  = false;

public:
    OutfitListTable(SosUiData &uiData, OutfitService &outfitService) : m_uiData(uiData), m_outfitService(outfitService) {}

    void OnRefresh();

    void Draw();

    auto GetEditingOutfit() -> EditingOutfit & { return editing_; }

private:
    // refresh, filterer, favorite checkbox...
    void DrawToolWidgets();
    void DrawOutfitTable();
    void DrawOutfitTableContent();
    void DrawCreateOutfitPopup(const char *name);

    void OnAcceptEditOutfit(const EditingOutfit &lastEdit, const EditingOutfit &editingOutfit) const;
    // check MultiSelection and set all selected outfit to favorite
    void OnAcceptSetFavoriteOutfits(bool toFavorite);
    void DeleteAllSelectOutfits();
};
} // namespace SosGui
