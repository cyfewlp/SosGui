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
#include "util/utils.h"

#include <array>
#include <functional>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace SosGui
{
void OutfitListTable::OnRefresh()
{
    editing_ = UNTITLED_OUTFIT;
    multi_selection_.Clear();
    outfit_name_buffer_[0] = '\0';
}

void OutfitListTable::Draw()
{
    if (ImGui::BeginChild(Translate1("Panels.Outfit.Title"), {}, ImGuiEx::ChildFlags().AutoResizeX()))
    {
        DrawToolWidgets();
        DrawOutfitTable();
    }
    ImGui::EndChild();
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
        spawn([&] { return m_outfitService.GetOutfitList(); });
        spawn([&] { return m_outfitService.GetAllFavoriteOutfits(); });
    }
    ImGui::SetItemTooltip("%s", Translate1("Panels.Outfit.Refresh"));

    ImGui::SameLine();
    ImGui::Checkbox(Translate1("Panels.Outfit.ShowFavorites"), &show_favorites_);

    ImGui::AlignTextToFramePadding();
    ImGuiUtil::IconButton(ICON_SEARCH);
    ImGui::SameLine(0.F, 0.F);

    ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
    ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
    name_filterer_.Draw("##filterer", -FLT_MIN);
    ImGui::PopItemFlag();
}

void OutfitListTable::DrawOutfitTable()
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
        DrawOutfitTableContent();
        ImGui::EndTable();
    }
}

namespace
{

auto DrawConfirmDeleteOutfitsPopup(const char *name, MultiSelection &selection, const std::vector<SosUiOutfit> &outfits) -> bool
{
    bool confirmed = false;
    bool open      = true;
    if (ImGui::BeginPopupModal(name, &open, ImGuiEx::WindowFlags().AlwaysAutoResize()))
    {
        void   *it = nullptr;
        ImGuiID id = 0;
        if (selection.GetNextSelectedItem(&it, &id) && id < outfits.size())
        {
            if (selection.Size > 1)
            {
                ImGuiUtil::Text(util::TranslateEx("Panels.OutfitEdit.DeleteMultiple", selection.Size));
                ImGuiUtil::Text(outfits[id].GetName());
                constexpr int maxShowCount   = 10;
                int           remainingCount = maxShowCount;
                while (--remainingCount > 0 && selection.GetNextSelectedItem(&it, &id))
                {
                    ImGuiUtil::Text(outfits[id].GetName());
                }
                if (remainingCount == 0)
                {
                    ImGuiUtil::Text(util::TranslateEx("Panels.OutfitEdit.DeleteMultipleAdditionalTips", selection.Size - maxShowCount));
                }
            }
            else
            {
                ImGuiUtil::Text(util::TranslateEx("Panels.OutfitEdit.DeleteOne", outfits[id].GetName()));
            }
            confirmed = Popup::DrawActionButtons();
        }
        else
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    return confirmed;
}
} // namespace

void OutfitListTable::DrawOutfitTableContent()
{
    constexpr const char *OUTFIT_NAME_INPUT_LABEL            = "##editableOutfitNameId";
    constexpr const char *CONFIRM_DELETE_OUTFITS_POPUP_TITLE = "Delete";

    ImGui::TableSetupScrollFreeze(1, 1);
    ImGui::TableSetupColumn("##Number", ImGuiEx::TableColumnFlags().NoSort().WidthFixed(), 56);
    ImGui::TableSetupColumn(Translate1("Panels.Outfit.Title"), ImGuiEx::TableColumnFlags().DefaultSort());
    ImGui::TableHeadersRow();

    static bool ascend = true;
    ImGuiUtil::may_update_table_sort_dir(ascend);

    auto OnRename = [this](const ImGuiID inputId, const std::string &outfitName) -> void {
        active_input_id_ = inputId;
        outfitName.copy(outfit_name_buffer_.data(), outfit_name_buffer_.size());
        ImGui::ActivateItemByID(inputId);
    };

    const std::vector<SosUiOutfit> &outfits = m_uiData.GetOutfitContainer().get_all();

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(outfits.size()));
    constexpr auto msFlags = ImGuiEx::MultiSelectFlags().NoSelectAll().BoxSelect1d().ClearOnEscape().ClearOnClickVoid();
    auto          *msIO    = multi_selection_.Begin(msFlags, clipper.ItemsCount);

    multi_selection_.ApplyRequests(msIO);
    if (msIO->RangeSrcItem != -1)
    {
        clipper.IncludeItemByIndex(static_cast<int>(msIO->RangeSrcItem));
    }
    const bool reverse          = !ascend;
    const int  step             = reverse ? -1 : 1;
    bool       wantDeleteOutfit = false;
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
            if (!pass_filter(outfit)) continue;

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
                    spawn([&] { return m_outfitService.SetOutfitIsFavorite(outfit.GetId(), outfit.GetName(), !outfit.IsFavorite()); });
                }

                ImGui::SameLine();
                ImGui::Text("%u", index + 1);
            }

            if (ImGui::TableNextColumn()) // outfit name column
            {
                if (const auto thisInputId = ImGui::GetID(OUTFIT_NAME_INPUT_LABEL); active_input_id_ == thisInputId)
                {
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::InputTextWithHint(OUTFIT_NAME_INPUT_LABEL, "rename outfit", outfit_name_buffer_.data(), outfit_name_buffer_.size());

                    if (ImGui::IsItemDeactivated())
                    {
                        std::string_view newNameSv(outfit_name_buffer_.data());
                        if (!newNameSv.empty() && newNameSv != outfit.GetName())
                        {
                            logger::info("Rename outfit {} to {}", outfit.GetName(), outfit_name_buffer_.data());
                            spawn([&] { return m_outfitService.RenameOutfit(outfit.GetId(), outfit.GetName(), std::string(newNameSv)); });
                        }
                        outfit_name_buffer_[0] = '\0';
                        active_input_id_       = 0;
                    }
                }
                else
                {
                    bool wantRename = false;
                    ImGui::SetNextItemSelectionUserData(index);
                    if (constexpr auto flags = ImGuiEx::SelectableFlags().AllowDoubleClick().AllowOverlap().SpanAllColumns();
                        ImGui::Selectable(outfit.GetName().c_str(), multi_selection_.Contains(uIndex), flags))
                    {
                        const EditingOutfit currentEditing(outfit);
                        OnAcceptEditOutfit(editing_, currentEditing);
                        editing_ = currentEditing;
                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        {
                            wantRename = true;
                        }
                    }

                    if (ImGui::BeginPopupContextItem())
                    {
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
                        if (multi_selection_.Size == 1 && ImGui::MenuItem(Translate1("Panels.Outfit.Rename")))
                        {
                            wantRename = true;
                        }
                        if (ImGui::MenuItem(Translate1("Delete")))
                        {
                            wantDeleteOutfit = true;
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
    multi_selection_.ApplyRequests(ImGui::EndMultiSelect());
    if (multi_selection_.Size == 0)
    {
        editing_ = UNTITLED_OUTFIT;
    }

    if (wantDeleteOutfit && multi_selection_.Size > 0)
    {
        ImGui::OpenPopup(CONFIRM_DELETE_OUTFITS_POPUP_TITLE);
    }

    // draw popup out of loop to avoid call multiple draw command.
    if (DrawConfirmDeleteOutfitsPopup(CONFIRM_DELETE_OUTFITS_POPUP_TITLE, multi_selection_, m_uiData.GetOutfitContainer().get_all()))
    {
        DeleteAllSelectOutfits();
    }
}

void OutfitListTable::DrawCreateOutfitPopup(const char *name)
{
    bool open = true;
    if (ImGui::BeginPopupModal(name, &open))
    {
        if (ImGui::IsWindowAppearing())
        {
            ImGui::SetKeyboardFocusHere();
        }

        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::InputTextWithHint(
            "##CreateNewOutfit",
            Translate1("Panels.Outfit.CreateHint"),
            outfit_name_buffer_.data(),
            outfit_name_buffer_.size(),
            ImGuiEx::InputTextFlags().AutoSelectAll()
        );
        ImGui::PopItemWidth();

        ImGui::BeginDisabled(outfit_name_buffer_[0] == '\0');
        if (ImGui::Button(Translate1("Panels.Outfit.Create")))
        {
            spawn([&] { return m_outfitService.CreateOutfit(std::string(outfit_name_buffer_.data())); });
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button(Translate1("Panels.Outfit.CreateFromWorn")))
        {
            spawn([&] { return m_outfitService.CreateOutfitFromWorn(std::string(outfit_name_buffer_.data())); });
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();

        ImGui::EndPopup();
    }
}

auto OutfitListTable::pass_filter(const SosUiOutfit &outfit) -> bool
{
    const auto &name = outfit.GetName();
    return name_filterer_.PassFilter(name.c_str(), name.c_str() + name.size()) && // filter by name
           (!show_favorites_ || outfit.IsFavorite());                             // or only favorites if checked
}

void OutfitListTable::OnAcceptEditOutfit(const EditingOutfit &lastEdit, const EditingOutfit &editingOutfit) const
{
    if (!editingOutfit.IsUntitled())
    {
        spawn([&] { return m_outfitService.GetOutfitArmors(editingOutfit.GetId(), editingOutfit.GetName()); });
        spawn([&] { return m_outfitService.GetSlotPolicy(editingOutfit.GetId(), editingOutfit.GetName()); });
    }

    // m_editPanel.OnSelectOutfit(lastEdit, editingOutfit); TODO
}

// ReSharper disable once CppDFAUnreachableFunctionCall
void OutfitListTable::OnAcceptSetFavoriteOutfits(const bool toFavorite)
{
    const auto &outfitContainer = m_uiData.GetOutfitContainer();
    void       *it              = nullptr;
    ImGuiID     selectedRank; // must be name rank;

    while (multi_selection_.GetNextSelectedItem(&it, &selectedRank))
    {
        const auto &outfit = outfitContainer.get_all()[selectedRank];
        spawn([&] { return m_outfitService.SetOutfitIsFavorite(outfit.GetId(), outfit.GetName(), toFavorite); });
    }
    multi_selection_.Clear();
}

// ReSharper disable once CppDFAUnreachableFunctionCall
void OutfitListTable::DeleteAllSelectOutfits()
{
    if (multi_selection_.Size <= 0) return;

    void   *it = nullptr;
    ImGuiID index;

    auto &outfits = m_uiData.GetOutfitContainer().get_all();
    while (multi_selection_.GetNextSelectedItem(&it, &index))
    {
        const auto &outfit = outfits[index];
        spawn([&] { return m_outfitService.DeleteOutfit(outfit.GetId(), outfit.GetName()); });
    }
    multi_selection_.Clear();
}
} // namespace SosGui
