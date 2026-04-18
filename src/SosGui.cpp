#include "SosGui.h"

#include "WCharUtils.h"
#include "fonts/FontManager.h"
#include "gui/icon.h"
#include "i18n/translator_manager.h"
#include "imgui.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/imgui_manager.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "log.h"
#include "path_utils.h"
#include "task.h"
#include "util/ImGuiUtil.h"
#include "util/utils.h"

#include <windows.h>

namespace SosGui
{

SosGuiWindow::~SosGuiWindow()
{
    i18n::SetTranslator(nullptr);
}

auto SosGuiWindow::Init(HWND hWnd, const RE::BSGraphics::RendererData &renderData) -> void
{
    constexpr std::string_view ICON_FONT = "lucide-icons.ttf";

    auto *device  = reinterpret_cast<ID3D11Device *>(renderData.forwarder);
    auto *context = reinterpret_cast<ID3D11DeviceContext *>(renderData.context);
    ImGuiEx::Initialize(hWnd, device, context);
    if (const auto defaultFontFilePath = Fonts::GetDefaultFontFilePath(); !defaultFontFilePath.empty())
    {
        (void)ImGuiEx::AddPrimaryFont({WCharUtils::ToString(defaultFontFilePath)}, {});
    }
    (void)ImGuiEx::AddFont(utils::GetInterfaceFile(ICON_FONT));

    ImGui::GetIO().ConfigNavEscapeClearFocusWindow = true;

    ImGui::StyleColorsDark();
}

auto SosGuiWindow::ShutDown() -> void
{
    ImGuiEx::Shutdown();
}

auto SosGuiWindow::refresh_actors_outfits() -> void
{
    spawn([&] { return OutfitService::RefreshAllActorsAutoSwitchOutfit(); });
    spawn([&] { return OutfitService::RefreshAllActorsActiveOutfit(); });
}

auto SosGuiWindow::Refresh() const -> void
{
    spawn([&] { return m_dataCoordinator.Refresh(); });
}

auto SosGuiWindow::OnPostDisplay() -> void
{
    ImGuiEx::NewFrame();

    Draw();

    ImGuiEx::Render();
    ImGuiEx::EndFrame();
}

auto SosGuiWindow::Draw() -> void
{
    MainMenuBar();
    ErrorNotifier::GetInstance().Show();

    if (!showing_) return;

    ZoneScopedN(__FUNCTION__);
    ImGui::SetNextWindowPos({600.0F, 200.F}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({1024.0F, 680.0F}, ImGuiCond_FirstUseEver);
    static bool show_character_edit_panel = true;
    const auto &plugin_name               = SKSE::PluginDeclaration::GetSingleton()->GetName();

    const auto outfit_editing_window_title = std::format("{} - {}", outfit_edit_panel_.get_sub_title(), plugin_name);
    const auto window_title                = show_character_edit_panel ? plugin_name : outfit_editing_window_title;
    if (ImGui::Begin(window_title.data(), &showing_))
    {
        if (ImGui::BeginChild("NavRail", {}, ImGuiEx::ChildFlags().Borders().AutoResizeX()))
        {
            constexpr auto padding_lines = 5;
            ImGui::Dummy({1.0F, ImGui::GetTextLineHeight() * padding_lines});
            if (ImGuiUtil::IconButton(ICON_USERS))
            {
                show_character_edit_panel = true;
            }
            if (ImGuiUtil::IconButton(ICON_SHIRT))
            {
                show_character_edit_panel = false;
            }
        }
        ImGui::EndChild();

        ImGui::SameLine(0.F, 0.F);
        if (ImGui::BeginChild("Panel"))
        {
            if (show_character_edit_panel)
            {
                character_edit_panel_.draw(ui_data_, m_dataCoordinator, outfit_service_);
            }
            else
            {
                outfit_edit_panel_.draw(ui_data_.outfit_container);
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void SosGuiWindow::MainMenuBar()
{
    if (!ImGui::BeginMainMenuBar())
    {
        return;
    }
    MainMenuAction main_menu_action = MainMenuAction::none;
    if (ImGui::BeginMenu(Translate1("ToolBar.File")))
    {
        bool enabled = ui_data_.enabled;
        if (ImGui::Checkbox(Translate1("Enabled"), &enabled))
        {
            spawn([&] { return m_dataCoordinator.RequestEnable(enabled); });
        }

        enabled = ui_data_.quick_slot_enabled;
        if (ImGui::Checkbox(Translate1("ToolBar.QuickSlots"), &enabled))
        {
            if (EnableQuickSlot(enabled))
            {
                ui_data_.quick_slot_enabled = enabled;
            }
        }
        ImGui::SetItemTooltip("%s", Translate1("ToolBar.QuickSlotsToolTip"));

        ImGuiUtil::Icon(ICON_FILE_PLUS_CORNER);
        ImGui::SameLine();
        if (ImGui::MenuItem(Translate1("Panels.Outfit.CreateNew"), "Ctrl+ N"))
        {
            main_menu_action = MainMenuAction::create_outfit;
        }
        if (ImGui::MenuItem(Translate1("ToolBar.Import")))
        {
            OnImportSettings();
            spawn([&] { return m_dataCoordinator.RequestImportSettings(); });
        }

        if (ImGui::MenuItem(Translate1("ToolBar.Export")))
        {
            spawn([&] { return m_dataCoordinator.RequestExportSettings(); });
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem(Translate1("ToolBar.ShowOrHide")))
    {
        showing_ = !showing_;
    }
    if (ImGui::MenuItem(Translate1("ToolBar.RefreshActorsOutfit")))
    {
        refresh_actors_outfits();
    }
    if (ImGui::MenuItem(Translate1("ToolBar.Close")))
    {
        auto *messageQueue = RE::UIMessageQueue::GetSingleton();
        messageQueue->AddMessage("SosGuiMenu", RE::UI_MESSAGE_TYPE::kHide, nullptr);
    }
    if (ImGui::MenuItem(Translate1("About")))
    {
        ImGui::OpenPopup("A Extra GUI for SkyrimOutfitSystemRE");
    }

    Popup::DrawAboutPopup("A Extra GUI for SkyrimOutfitSystemRE");

    ImGui::EndMainMenuBar();

    outfit_edit_panel_.on_main_menu_action(main_menu_action);
}

void SosGuiWindow::OnImportSettings()
{
    outfit_edit_panel_.on_refresh();
}

auto SosGuiWindow::EnableQuickSlot(const bool enable) -> bool
{
    const auto &player = RE::PlayerCharacter::GetSingleton();
    if (auto *spell = RE::TESForm::LookupByEditorID<RE::SpellItem>(SOS_SPELL_EDITOR_ID); spell != nullptr)
    {
        enable ? player->AddSpell(spell) : player->RemoveSpell(spell);
        return true;
    }
    return false;
}
} // namespace SosGui
