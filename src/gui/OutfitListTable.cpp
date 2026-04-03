#include "gui/OutfitListTable.h"

#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "gui/UiSettings.h"
#include "gui/icon.h"
#include "gui/widgets.h"
#include "i18n/translator_manager.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "util/ImGuiUtil.h"

#include <array>
#include <boost/optional/detail/optional_reference_spec.hpp>
#include <functional>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace SosGui
{
void OutfitListTable::Show()
{
    BaseGui::Show();
    OnAcceptEditOutfit(m_wantEdit, m_wantEdit); // trigger OutfitEditPanel#UpdateWindowTitle
}

void OutfitListTable::Focus()
{
    ImGui::SetWindowFocus(Translate1("Panels.Outfit.Title"));
    BaseGui::Focus();
}

void OutfitListTable::OnRefresh()
{
    m_wantEdit = UNTITLED_OUTFIT;
    m_outfitMultiSelection.Clear();
    m_outfitFilterInput.Clear();
}

void OutfitListTable::Cleanup()
{
    m_wantEdit = UNTITLED_OUTFIT;
    m_outfitMultiSelection.Clear();
    m_outfitFilterInput.Clear();
}

void OutfitListTable::Draw(RE::Actor *editingActor)
{
    if (!IsShowing())
    {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(DEFAULT_OUTFIT_LIST_WINDOW_POS_X, DEFAULT_WINDOW_POS_Y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(DEFAULT_WINDOW_WIDTH_SMALL, DEFAULT_WINDOW_HEIGHT), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(Translate1("Panels.Outfit.Title"), &m_show, ImGuiEx::WindowFlags()))
    {
        DrawToolWidgets();
        DrawOutfitTable(editingActor);
    }
    ImGui::End();
}

void OutfitListTable::DrawToolWidgets()
{
    constexpr const char *CREATE_OUTFIT_POPUP_TITLE = "Create Outfit";
    if (ImGuiUtil::IconButton(ICON_FILE_PLUS_CORNER))
    {
        ImGui::OpenPopup(CREATE_OUTFIT_POPUP_TITLE);
    }
    ImGui::SameLine();
    ImGuiUtil::Text("Create");
    DrawCreateOutfitPopup(CREATE_OUTFIT_POPUP_TITLE);

    ImGui::SameLine();
    auto &outfitContainer = m_uiData.GetOutfitContainer();
    if (ImGuiUtil::IconButton(ICON_REFRESH_CW))
    {
        OnRefresh();
        outfitContainer.get_all().clear();
        +[&] {
            return m_outfitService.GetOutfitList();
        };
        +[&] {
            return m_outfitService.GetAllFavoriteOutfits();
        };
    }
    ImGui::SetItemTooltip("%s", Translate1("Panels.Outfit.Refresh"));

    ImGui::SameLine();
    if (ImGui::Checkbox(Translate1("Panels.Outfit.ShowFavorites"), &show_favorites_))
    {
        m_outfitFilterInput.dirty = true;
    }

    ImGui::AlignTextToFramePadding();
    ImGuiUtil::IconButton(ICON_SEARCH);
    ImGui::SameLine(0.F, 0.F);

    ImGui::PushItemWidth(-FLT_MIN);
    m_outfitFilterInput.Draw("##filter", Translate1("Panels.Outfit.Filter"));
    ImGui::PopItemWidth();
}

void OutfitListTable::DrawOutfitTable(RE::Actor *editingActor)
{
    const auto styleGuard = ImGuiEx::StyleGuard().Style<ImGuiStyleVar_CellPadding>(ImVec2(10, 5));
    if (ImGui::BeginTable(
            "##OutfitLists",
            2,
            ImGuiEx::TableFlags()
                .Borders()
                .Resizable()
                .Hideable()
                .Reorderable()
                .Sortable()
                .SizingStretchProp()
                .ScrollY()
                .NoHostExtendX()
                .ContextMenuInBody()
        ))
    {
        DrawOutfitTableContent(editingActor);
        ImGui::EndTable();
    }
}

namespace
{

auto DrawConfirmDeleteOutfitsPopup(const char *name, MultiSelection &selection, const SosUiOutfit *clickedOutfit) -> bool
{
    bool confirmed = false;
    if (ImGui::BeginPopupModal(name, nullptr))
    {
        if (selection.Size > 0)
        {
            const auto tipsMessage = selection.Size == 1 ? clickedOutfit->GetName() : std::format("{} outfit", selection.Size);
            ImGuiUtil::Text(tipsMessage);

            ImGuiUtil::Text(std::format("{} \"{}\"?", Translate("Panels.OutfitEdit.Delete"), tipsMessage));
            confirmed = Popup::DrawActionButtons();
        }
        else
        {
            assert(false && "Want delete outfit but no select any outfit!");
        }
        ImGui::EndPopup();
    }
    return confirmed;
}
} // namespace

#define INVALID_INDEX SIZE_MAX

void OutfitListTable::DrawOutfitTableContent(RE::Actor *editingActor)
{
    constexpr const char *OUTFIT_NAME_INPUT_LABEL            = "##editableOutfitNameId";
    constexpr const char *CONFIRM_DELETE_OUTFITS_POPUP_TITLE = "Delete";

    ImGui::TableSetupScrollFreeze(1, 1);
    ImGui::TableSetupColumn("##Number", ImGuiEx::TableColumnFlags().NoSort().WidthFixed(), 56);
    ImGui::TableSetupColumn(Translate1("Panels.Outfit.Title"), ImGuiEx::TableColumnFlags().DefaultSort());
    ImGui::TableHeadersRow();

    static bool ascend = true;
    ImGuiUtil::may_update_table_sort_dir(ascend);

    auto OnRename = [this](const ImGuiID inputId, const std::string &outfitName) {
        m_editingInputId = inputId;
        outfitName.copy(m_outfitNameBuffer.data(), m_outfitNameBuffer.size());
        ImGui::ActivateItemByID(inputId);
    };

    const std::vector<SosUiOutfit> &outfits = m_uiData.GetOutfitContainer().get_all();

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(outfits.size()));
    constexpr auto msFlags = ImGuiEx::MultiSelectFlags().NoSelectAll().BoxSelect1d().ClearOnEscape().ClearOnClickVoid();
    auto          *msIO    = m_outfitMultiSelection.Begin(msFlags, clipper.ItemsCount);

    m_outfitMultiSelection.ApplyRequests(msIO);
    if (msIO->RangeSrcItem != -1)
    {
        clipper.IncludeItemByIndex(static_cast<int>(msIO->RangeSrcItem));
    }
    const bool         reverse          = !ascend;
    const int          step             = reverse ? -1 : 1;
    bool               wantDeleteOutfit = false;
    const SosUiOutfit *targetOutfit     = nullptr;
    while (clipper.Step())
    {
        auto       start     = clipper.DisplayStart;
        auto       end       = clipper.DisplayEnd;
        const auto itemCount = end - start;
        if (reverse)
        {
            start = static_cast<int>(outfits.size()) - 1 - start;
            end   = start - itemCount;
        }
        for (auto index = start; index != end; index += step)
        {
            const auto  uIndex = static_cast<ImGuiID>(index);
            const auto &outfit = outfits[uIndex];
            if (show_favorites_ && !outfit.IsFavorite()) continue;

            ImGui::PushID(index);
            ImGui::TableNextRow();
            if (ImGui::TableNextColumn()) // number column
            {
                bool clicked = false;
                if (outfit.IsFavorite())
                {
                    clicked = ImGuiUtil::IconButton(ICON_HEART);
                    ImGui::SetItemTooltip("%s", Translate1("Panels.Outfit.UnmarkFavorite"));
                }
                else
                {
                    clicked = ImGuiUtil::IconButton(ICON_HEART_OFF);
                    ImGui::SetItemTooltip("%s", Translate1("Panels.Outfit.MarkFavorite"));
                }
                if (clicked)
                {
                    +[&] {
                        return m_outfitService.SetOutfitIsFavorite(outfit.GetId(), outfit.GetName(), !outfit.IsFavorite());
                    };
                }

                ImGui::SameLine();
                ImGui::Text("%u", index + 1);
            }

            if (ImGui::TableNextColumn()) // outfit name column
            {
                if (const auto thisInputId = ImGui::GetID(OUTFIT_NAME_INPUT_LABEL); m_editingInputId == thisInputId)
                {
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::InputTextWithHint(OUTFIT_NAME_INPUT_LABEL, "rename outfit", m_outfitNameBuffer.data(), m_outfitNameBuffer.size());

                    if (ImGui::IsItemDeactivated())
                    {
                        std::string_view newNameSv(m_outfitNameBuffer.data());
                        if (!newNameSv.empty() && newNameSv != outfit.GetName())
                        {
                            logger::info("Rename outfit {} to {}", outfit.GetName(), m_outfitNameBuffer.data());
                            +[&] {
                                return m_outfitService.RenameOutfit(outfit.GetId(), outfit.GetName(), std::string(newNameSv));
                            };
                        }
                        m_outfitNameBuffer[0] = '\0';
                        m_editingInputId      = 0;
                    }
                }
                else
                {
                    bool wantRename = false;
                    ImGui::SetNextItemSelectionUserData(index);
                    if (constexpr auto flags = ImGuiEx::SelectableFlags().AllowDoubleClick().AllowOverlap().SpanAllColumns();
                        ImGui::Selectable(outfit.GetName().c_str(), m_outfitMultiSelection.Contains(uIndex), flags))
                    {
                        const EditingOutfit currentEditing(outfit);
                        OnAcceptEditOutfit(m_wantEdit, currentEditing);
                        m_wantEdit = currentEditing;
                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        {
                            wantRename = true;
                        }
                    }

                    if (ImGui::BeginPopupContextItem())
                    {
                        const auto &outfitName = outfit.GetName();

                        ImGui::Separator();
                        if (ImGui::MenuItem(Translate1("Panels.Outfit.MarkFavorite")))
                        {
                            OnAcceptSetFavoriteOutfits(true);
                        }
                        if (ImGui::MenuItem(Translate1("Panels.Outfit.UnmarkFavorite")))
                        {
                            OnAcceptSetFavoriteOutfits(false);
                        }
                        ImGui::Separator();
                        if (m_outfitMultiSelection.Size == 1 && ImGui::MenuItem(Translate1("Panels.Outfit.Rename")))
                        {
                            wantRename = true;
                        }
                        if (ImGui::MenuItem(Translate1("Delete")))
                        {
                            wantDeleteOutfit = true;
                            targetOutfit     = &outfit;
                        }
                        ImGui::EndPopup();
                    }
                    if (wantRename)
                    {
                        OnRename(thisInputId, outfit.GetName());
                    }
                }
            }
            ImGui::PopID();
        }
    }
    m_outfitMultiSelection.ApplyRequests(ImGui::EndMultiSelect());

    if (wantDeleteOutfit)
    {
        ImGui::OpenPopup(CONFIRM_DELETE_OUTFITS_POPUP_TITLE);
    }

    // draw popup out of loop to avoid call multiple draw command.
    if (DrawConfirmDeleteOutfitsPopup(CONFIRM_DELETE_OUTFITS_POPUP_TITLE, m_outfitMultiSelection, targetOutfit))
    {
        DeleteAllSelectOutfits(targetOutfit);
    }
}

void OutfitListTable::DrawCreateOutfitPopup(const char *name)
{
    bool open = true;
    if (ImGui::BeginPopupModal(name, &open))
    {
        // Clear the active id that rename input item if exists.
        // It should unreachable.
        if (m_editingInputId != 0 && m_editingInputId == ImGui::GetActiveID())
        {
            ImGui::ClearActiveID();
        }

        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::InputTextWithHint("##CreateNewOutfit", Translate1("Panels.Outfit.CreateHint"), m_outfitNameBuffer.data(), m_outfitNameBuffer.size());
        ImGui::PopItemWidth();

        ImGui::BeginDisabled(m_outfitNameBuffer[0] == '\0');
        if (ImGui::Button(Translate1("Panels.Outfit.Create")))
        {
            +[&] {
                return m_outfitService.CreateOutfit(std::string(m_outfitNameBuffer.data()));
            };
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button(Translate1("Panels.Outfit.CreateFromWorn")))
        {
            +[&] {
                return m_outfitService.CreateOutfitFromWorn(std::string(m_outfitNameBuffer.data()));
            };
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();

        ImGui::EndPopup();
    }
}

void OutfitListTable::OnAcceptEditOutfit(const EditingOutfit &lastEdit, const EditingOutfit &editingOutfit) const
{
    if (!editingOutfit.IsUntitled())
    {
        +[&] {
            return m_outfitService.GetOutfitArmors(editingOutfit.GetId(), editingOutfit.GetName());
        };
        +[&] {
            return m_outfitService.GetSlotPolicy(editingOutfit.GetId(), editingOutfit.GetName());
        };
    }

    m_editPanel.OnSelectOutfit(lastEdit, editingOutfit);
}

// ReSharper disable once CppDFAUnreachableFunctionCall
void OutfitListTable::OnAcceptSetFavoriteOutfits(const bool toFavorite)
{
    const auto &outfitContainer = m_uiData.GetOutfitContainer();
    void       *it              = nullptr;
    ImGuiID     selectedRank; // must be name rank;

    while (m_outfitMultiSelection.GetNextSelectedItem(&it, &selectedRank))
    {
        const auto &outfit = outfitContainer.get_all()[selectedRank];
        +[&] {
            return m_outfitService.SetOutfitIsFavorite(outfit.GetId(), outfit.GetName(), toFavorite);
        };
    }
    m_outfitMultiSelection.Clear();
}

// ReSharper disable once CppDFAUnreachableFunctionCall
void OutfitListTable::DeleteAllSelectOutfits(const SosUiOutfit *clickedOutfit)
{
    if (m_outfitMultiSelection.Size <= 0) return;

    if (m_outfitMultiSelection.Size == 1)
    {
        +[&] {
            return m_outfitService.DeleteOutfit(clickedOutfit->GetId(), clickedOutfit->GetName());
        };
        return;
    }

    void   *it = nullptr;
    ImGuiID index; // must be name rank;

    auto &outfits = m_uiData.GetOutfitContainer().get_all();
    while (m_outfitMultiSelection.GetNextSelectedItem(&it, &index))
    {
        const auto &outfit = outfits[index];
        +[&] {
            return m_outfitService.DeleteOutfit(outfit.GetId(), outfit.GetName());
        };
    }
    m_outfitMultiSelection.Clear();
}
} // namespace SosGui
