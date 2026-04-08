#include "gui/OutfitListTable.h"

#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "gui/UiSettings.h"
#include "gui/icon.h"
#include "i18n/translator_manager.h"
#include "imgui.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "tracy/Tracy.hpp"
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

namespace
{

auto DrawConfirmDeleteOutfitsPopup(const char *name, ImGuiSelectionBasicStorage &selection, const std::vector<SosUiOutfit> &outfits) -> bool
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

void OutfitListTable::OnRefresh()
{
    editing_ = UNTITLED_OUTFIT;
    multi_selection_.Clear();
    outfit_name_buffer_[0] = '\0';
}

void OutfitListTable::Draw(const std::vector<SosUiOutfit> &outfits, OutfitService &outfitService)
{
    ZoneScopedN(__FUNCTION__);
    if (ImGui::BeginChild(Translate1("Panels.Outfit.Title"), {}, ImGuiEx::ChildFlags().AutoResizeX()))
    {
        DrawToolWidgets(outfitService);
        const auto styleGuard = ImGuiEx::StyleGuard().Style<ImGuiStyleVar_CellPadding>(ImVec2(10, 5));
        const auto flags =
            ImGuiEx::TableFlags().Borders().Resizable().Hideable().Reorderable().Sortable().SizingStretchProp().ScrollY().ContextMenuInBody();
        if (ImGui::BeginTable("##OutfitLists", 2, flags))
        {
            ImGui::TableSetupScrollFreeze(1, 1);
            ImGui::TableSetupColumn("##Number", ImGuiEx::TableColumnFlags().NoSort(), 56);
            ImGui::TableSetupColumn(Translate1("Panels.Outfit.Title"), ImGuiEx::TableColumnFlags().DefaultSort());
            ImGui::TableHeadersRow();

            if (auto *sortSpecs = ImGui::TableGetSortSpecs(); sortSpecs != nullptr)
            {
                if (sortSpecs->SpecsDirty && sortSpecs->SpecsCount > 0)
                {
                    const auto direction  = sortSpecs->Specs[0].SortDirection;
                    name_sort_ascend_     = direction == ImGuiSortDirection_Ascending;
                    sortSpecs->SpecsDirty = false;
                }
            }

            DrawOutfitTableContent(outfits, outfitService);
            ImGui::EndTable();
        }
    }
    ImGui::EndChild();

    if (editing_.GetId() != editing_id_)
    {
        const auto it = std::ranges::find_if(outfits, [this](const SosUiOutfit &outfit) { return outfit.GetId() == editing_id_; });
        if (it != outfits.end())
        {
            editing_ = *it;
            spawn([&] { return outfitService.GetSlotPolicy(editing_); });
        }
        else
        {
            editing_id_ = editing_.GetId();
        }
    }
}

void OutfitListTable::DrawToolWidgets(OutfitService &outfitService)
{
    constexpr const char *CREATE_OUTFIT_POPUP_TITLE = "Create Outfit";
    if (ImGuiUtil::IconButton(ICON_FILE_PLUS_CORNER))
    {
        ImGui::OpenPopup(CREATE_OUTFIT_POPUP_TITLE);
    }
    ImGui::SameLine();
    ImGuiUtil::Text("Create");
    DrawCreateOutfitPopup(CREATE_OUTFIT_POPUP_TITLE, outfitService);

    ImGui::SameLine();
    if (ImGuiUtil::IconButton(ICON_REFRESH_CW))
    {
        OnRefresh();
        spawn([&] { return outfitService.GetOutfitList(); });
        spawn([&] { return outfitService.GetAllFavoriteOutfits(); });
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

void OutfitListTable::DrawOutfitTableContent(const std::vector<SosUiOutfit> &outfits, OutfitService &outfitService)
{
    ZoneScopedN(__FUNCTION__);
    constexpr const char *OUTFIT_NAME_INPUT_LABEL            = "##editableOutfitNameId";
    constexpr const char *CONFIRM_DELETE_OUTFITS_POPUP_TITLE = "Delete";

    if (outfits.empty())
    {
        return;
    }

    auto OnRename = [this](const ImGuiID inputId, const std::string &outfitName) -> void {
        active_input_id_ = inputId;
        outfitName.copy(outfit_name_buffer_.data(), outfit_name_buffer_.size());
    };

    constexpr auto msFlags = ImGuiEx::MultiSelectFlags().NoSelectAll().BoxSelect1d().ClearOnEscape().ClearOnClickVoid();
    auto          *msIO    = ImGui::BeginMultiSelect(msFlags, multi_selection_.Size, static_cast<int>(outfits.size()));

    multi_selection_.ApplyRequests(msIO);
    const bool reverse          = !name_sort_ascend_;
    const int  start            = reverse ? static_cast<int>(outfits.size()) - 1 : 0;
    const int  end              = reverse ? 0 : static_cast<int>(outfits.size()) - 1;
    const int  step             = reverse ? -1 : 1;
    bool       wantDeleteOutfit = false;
    for (int index{start}; index != end; index += step)
    {
        const auto  uIndex = static_cast<ImGuiID>(index);
        const auto &outfit = outfits[uIndex];
        if (!pass_filter(outfit)) continue;

        ImGui::PushID(index);
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
                spawn([&] { return outfitService.SetOutfitIsFavorite(outfit.GetId(), outfit.GetName(), !outfit.IsFavorite()); });
            }

            ImGui::SameLine();
            ImGui::Text("%u", index + 1);
        }

        if (ImGui::TableNextColumn()) // outfit name column
        {
            if (const auto thisInputId = ImGui::GetID(OUTFIT_NAME_INPUT_LABEL); active_input_id_ == thisInputId)
            {
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::SetKeyboardFocusHere();
                ImGui::InputTextWithHint(OUTFIT_NAME_INPUT_LABEL, "rename outfit", outfit_name_buffer_.data(), outfit_name_buffer_.size());

                if (ImGui::IsItemDeactivated())
                {
                    std::string_view newNameSv(outfit_name_buffer_.data());
                    if (!newNameSv.empty() && newNameSv != outfit.GetName())
                    {
                        logger::info("Rename outfit {} to {}", outfit.GetName(), outfit_name_buffer_.data());
                        spawn([&] { return outfitService.RenameOutfit(outfit.GetId(), outfit.GetName(), std::string(newNameSv)); });
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
                    editing_id_ = outfit.GetId();
                    spawn([&] { return outfitService.GetOutfitArmors(outfit.GetId(), outfit.GetName()); });
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
                        OnAcceptSetFavoriteOutfits(true, outfits, outfitService);
                    }
                    if (ImGui::MenuItem(Translate1("Panels.Outfit.UnmarkFavorite")))
                    {
                        OnAcceptSetFavoriteOutfits(false, outfits, outfitService);
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
    if (DrawConfirmDeleteOutfitsPopup(CONFIRM_DELETE_OUTFITS_POPUP_TITLE, multi_selection_, outfits))
    {
        DeleteAllSelectOutfits(outfits, outfitService);
    }
}

void OutfitListTable::DrawCreateOutfitPopup(const char *name, OutfitService &outfitService)
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
            editing_ = UNTITLED_OUTFIT;
            spawn([&] { return outfitService.CreateOutfit(std::string(outfit_name_buffer_.data())); });
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button(Translate1("Panels.Outfit.CreateFromWorn")))
        {
            editing_ = UNTITLED_OUTFIT;
            spawn([&] { return outfitService.CreateOutfitFromWorn(std::string(outfit_name_buffer_.data())); });
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

// ReSharper disable once CppDFAUnreachableFunctionCall
void OutfitListTable::OnAcceptSetFavoriteOutfits(const bool toFavorite, const std::vector<SosUiOutfit> &outfits, OutfitService &outfitService)
{
    void   *it    = nullptr;
    ImGuiID index = 0;
    while (multi_selection_.GetNextSelectedItem(&it, &index))
    {
        const auto &outfit = outfits[index];
        spawn([&] { return outfitService.SetOutfitIsFavorite(outfit.GetId(), outfit.GetName(), toFavorite); });
    }
    multi_selection_.Clear();
}

// ReSharper disable once CppDFAUnreachableFunctionCall
void OutfitListTable::DeleteAllSelectOutfits(const std::vector<SosUiOutfit> &outfits, OutfitService &outfitService)
{
    if (multi_selection_.Size <= 0) return;
    editing_ = UNTITLED_OUTFIT;

    void   *it = nullptr;
    ImGuiID index;

    while (multi_selection_.GetNextSelectedItem(&it, &index))
    {
        const auto &outfit = outfits[index];
        spawn([&] { return outfitService.DeleteOutfit(outfit.GetId(), outfit.GetName()); });
    }
    multi_selection_.Clear();
}
} // namespace SosGui
