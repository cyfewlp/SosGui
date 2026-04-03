#pragma once

#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "gui/BaseGui.h"
#include "gui/OutfitEditPanel.h"
#include "popup/Popup.h"
#include "service/OutfitService.h"
#include "util/ImGuiUtil.h"

#include <string>

namespace SosGui
{
class OutfitContainer;

class OutfitListTable final : public BaseGui
{
    static constexpr int        MAX_OUTFIT_NAME_BYTES = 256;
    static inline auto          OUTFIT                = SosUiOutfit();
    static inline EditingOutfit UNTITLED_OUTFIT{OUTFIT};
    using DrawOutfitEntry  = std::function<void(const SosUiOutfit &, ImGuiID)>;
    using OutfitNameBuffer = std::array<char, MAX_OUTFIT_NAME_BYTES>;

    SosUiData               &m_uiData;
    OutfitService           &m_outfitService;
    OutfitEditPanel         &m_editPanel;
    EditingOutfit            m_wantEdit = UNTITLED_OUTFIT;
    MultiSelection           m_outfitMultiSelection;
    ImGuiUtil::DebounceInput m_outfitFilterInput{};
    ImGuiID                  m_editingInputId = 0;
    OutfitNameBuffer         m_outfitNameBuffer{};
    bool                     show_favorites_ = false;

public:
    OutfitListTable(SosUiData &uiData, OutfitService &outfitService, OutfitEditPanel &editPanel)
        : m_uiData(uiData), m_outfitService(outfitService), m_editPanel(editPanel)
    {
    }

    void Show() override;
    void Focus() override;
    void OnRefresh() override;
    void Cleanup() override;

    void Draw(RE::Actor *editingActor);

    auto GetEditingOutfit() -> EditingOutfit & { return m_wantEdit; }

private:
    // refresh, filterer, favorite checkbox...
    void DrawToolWidgets();
    void DrawOutfitTable(RE::Actor *editingActor);
    void DrawOutfitTableContent(RE::Actor *editingActor);
    void DrawCreateOutfitPopup(const char *name);
    bool ConfirmDeleteOutfitPopup(const char *popupName, const SosUiOutfit *outfit);

    void OnAcceptEditOutfit(const EditingOutfit &lastEdit, const EditingOutfit &editingOutfit) const;
    // check MultiSelection and set all selected outfit to favorite
    void OnAcceptSetFavoriteOutfits(bool toFavorite);
    void DeleteAllSelectOutfits(const SosUiOutfit *clickedOutfit);
};
} // namespace SosGui
