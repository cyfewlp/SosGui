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

enum class MainMenuAction : std::uint8_t;

class OutfitListTable final
{
    static constexpr int        MAX_OUTFIT_NAME_BYTES = 256;
    static inline EditingOutfit UNTITLED_OUTFIT;
    using DrawOutfitEntry  = std::function<void(const SosUiOutfit &, ImGuiID)>;
    using OutfitNameBuffer = std::array<char, MAX_OUTFIT_NAME_BYTES>;

public:
    void OnRefresh();

    void on_main_menu_action(MainMenuAction main_menu_action);
    void Draw(const std::vector<SosUiOutfit> &outfits, OutfitService &outfitService);

    auto GetEditingOutfit() -> EditingOutfit & { return editing_; }

private:
    enum class MenuAction : std::uint8_t;

    // refresh, filterer, favorite checkbox...
    void DrawToolWidgets(OutfitService &outfitService);
    void DrawOutfitTableContent(const std::vector<SosUiOutfit> &outfits, OutfitService &outfitService);
    void draw_outfit_row(uint32_t index, const SosUiOutfit &outfit, MenuAction &menu_action, OutfitService &outfit_service);
    void DrawCreateOutfitPopup(const std::vector<SosUiOutfit> &outfits, const char *name, OutfitService &outfitService);

    auto pass_filter(const SosUiOutfit &outfit) -> bool;

    // check MultiSelection and set all selected outfit to favorite
    void OnAcceptSetFavoriteOutfits(bool toFavorite, const std::vector<SosUiOutfit> &outfits, OutfitService &outfitService);
    void DeleteAllSelectOutfits(const std::vector<SosUiOutfit> &outfits, OutfitService &outfitService);

    enum class MenuAction : std::uint8_t
    {
        none,
        delete_all,
        mark_favorite,
        mark_unfavorite,
    };

    enum class PopupStatus : std::uint8_t
    {
        none,
        open_delete,
        open_create,
    };

    ImGuiTextFilter            name_filterer_;
    OutfitId                   editing_id_ = INVALID_OUTFIT_ID;
    EditingOutfit              editing_    = UNTITLED_OUTFIT;
    ImGuiSelectionBasicStorage multi_selection_;
    OutfitNameBuffer           outfit_name_buffer_{};
    ImGuiID                    active_input_id_          = 0;
    // cached field, be used to ImGui list clipper, -1 means the list need to evaluate item count.
    int                        view_item_count_          = -1;
    bool                       show_conflict_name_error_ = false;
    bool                       show_favorites_           = false;
    bool                       name_sort_ascend_         = true;
    PopupStatus                popup_status_             = PopupStatus::none;
};
} // namespace SosGui
