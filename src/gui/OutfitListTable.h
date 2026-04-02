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

struct OutfitModifyRequest
{
    explicit OutfitModifyRequest(const SosUiOutfit *const outfit) : outfit(outfit) {}

    [[nodiscard]] auto IsAcceptedAny() const -> bool { return acceptedRename || acceptedDelete; }

    [[nodiscard]] auto IsAcceptedRename() const -> bool { return acceptedRename; }

    [[nodiscard]] auto IsAcceptedDelete() const -> bool { return acceptedDelete; }

    [[nodiscard]] auto GetOutfit() const -> const SosUiOutfit * { return outfit; }

    void AcceptRename() { acceptedRename = true; }

    void AcceptDelete() { acceptedDelete = true; }

private:
    const SosUiOutfit *outfit            = nullptr;
    bool               acceptedRename    = false; ///< is accepted rename outfit?
    bool               acceptedDelete    = false; ///< is accepted delete all select outfits?
};

class OutfitListTable final : public BaseGui
{
    static constexpr int        MAX_OUTFIT_NAME_BYTES = 256;
    static inline auto          OUTFIT                = SosUiOutfit();
    static inline EditingOutfit UNTITLED_OUTFIT{OUTFIT};
    using DrawOutfitEntry  = std::function<void(const SosUiOutfit *, ImGuiID)>;
    using OutfitNameBuffer = std::array<char, MAX_OUTFIT_NAME_BYTES>;

    struct OutfitDebounceInput final : ImGuiUtil::DebounceInput
    {
        std::vector<const SosUiOutfit *> viewData;

        OutfitDebounceInput() = default;

        void OnInput() override
        {
            viewData.clear();
            DebounceInput::OnInput();
        }

        void OnUpdate(const OutfitList &outfitList, bool onlyFavorites);

        void Clear() override
        {
            DebounceInput::Clear();
            viewData.clear();
        }
    };

    SosUiData          &m_uiData;
    OutfitService      &m_outfitService;
    OutfitEditPanel    &m_editPanel;
    EditingOutfit       m_wantEdit = UNTITLED_OUTFIT;
    MultiSelection      m_outfitMultiSelection;
    OutfitDebounceInput m_outfitFilterInput{};
    ImGuiID             m_editingInputId = 0;
    OutfitNameBuffer    m_outfitNameBuffer{};

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
    void OnToggleFavorite(std::vector<const SosUiOutfit *> &viewData, size_t index) const;
    void DrawCreateOutfitPopup(const char *name);

    static void PreDrawOutfits(ImGuiListClipper &clipper, MultiSelection &selection);
    static void PostDrawOutfits(MultiSelection &selection);
    void        DrawOutfitTableContent(std::vector<const SosUiOutfit *> outfitView, bool ascend, const DrawOutfitEntry &drawOutfitEntry);
    bool        ConfirmDeleteOutfitPopup(const char *popupName, const SosUiOutfit *outfit);

    /**
     * open a context menu if user right-clicks current row
     * @return true if the context menu is open.
     */
    void OpenContextMenu(RE::Actor *editingActor, const SosUiOutfit *clickedOutfit, OutfitModifyRequest &contextMenu);

    void OnAcceptEditOutfit(const EditingOutfit &lastEdit, const EditingOutfit &editingOutfit) const;
    void OnAcceptActiveOutfit(RE::Actor *editingActor, OutfitId id, const std::string &outfitName) const;
    // check MultiSelection and set all selected outfit to favorite
    void OnAcceptSetFavoriteOutfits(bool toFavorite);
    void DeleteAllSelectOutfits(const SosUiOutfit *clickedOutfit);
};
} // namespace SosGui
