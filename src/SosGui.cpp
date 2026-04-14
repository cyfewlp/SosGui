#include "SosGui.h"

#include "WCharUtils.h"
#include "fonts/FontManager.h"
#include "gui/UiSettings.h"
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
#include "util/UiSettingsLoader.h"
#include "util/utils.h"

#include <windows.h>

namespace SosGui
{
SosGuiWindow::SosGuiWindow() : m_outfitService(m_uiData), m_dataCoordinator(m_uiData, m_outfitService), outfit_edit_panel_(m_uiData, m_outfitService)
{
}

SosGuiWindow::~SosGuiWindow()
{
    m_isShowPanels = true;
    i18n::SetTranslator(nullptr);
}

auto SosGuiWindow::Init(HWND hWnd, const RE::BSGraphics::RendererData &renderData) -> void
{
    auto *uiSetting = ::SosGui::Settings::UiSettings::GetInstance();
    Settings::Load(*uiSetting);

    auto *device  = reinterpret_cast<ID3D11Device *>(renderData.forwarder);
    auto *context = reinterpret_cast<ID3D11DeviceContext *>(renderData.context);
    ImGuiEx::Initialize(hWnd, device, context);
    if (const auto defaultFontFilePath = Fonts::GetDefaultFontFilePath(); !defaultFontFilePath.empty())
    {
        (void)ImGuiEx::AddPrimaryFont({WCharUtils::ToString(defaultFontFilePath)}, {});
    }
    (void)ImGuiEx::AddFont(utils::GetInterfaceFile(Settings::UiSettings::ICON_FONT));

    ImGui::StyleColorsDark();
}

auto SosGuiWindow::ShutDown() -> void
{
    auto *uiSetting = Settings::UiSettings::GetInstance();
    Settings::Save(*uiSetting);
    ImGuiEx::Shutdown();
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

    if (!m_isShowPanels)
    {
        return;
    }
    try
    {
        character_edit_panel_.Draw(m_uiData, m_dataCoordinator, m_outfitService);
        outfit_edit_panel_.Draw();
    }
    catch (const std::exception &e)
    {
        logger::LogStacktrace();
        ErrorNotifier::GetInstance().Error(e.what());
    }
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
        bool enabled = m_uiData.enabled;
        if (ImGui::Checkbox(Translate1("Enabled"), &enabled))
        {
            spawn([&] { return m_dataCoordinator.RequestEnable(enabled); });
        }

        enabled = m_uiData.quick_slot_enabled;
        if (ImGui::Checkbox(Translate1("ToolBar.QuickSlots"), &enabled))
        {
            if (EnableQuickSlot(enabled))
            {
                m_uiData.quick_slot_enabled = enabled;
            }
        }
        ImGui::SetItemTooltip("%s", Translate1("ToolBar.QuickSlotsToolTip"));

        ImGuiUtil::Icon(ICON_FILE_PLUS_CORNER);
        ImGui::SameLine();
        if (ImGui::MenuItem(Translate1("Panels.Outfit.Create"), "Ctrl+ N"))
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
        m_isShowPanels = !m_isShowPanels;
    }
    if (ImGui::MenuItem(Translate1("ToolBar.RefreshPlayerArmor")))
    {
        util::RefreshActorArmor(RE::PlayerCharacter::GetSingleton());
    }
    if (ImGui::MenuItem(Translate1("ToolBar.Settings")))
    {
        ImGui::OpenPopup(Translate1("ToolBar.Settings"));
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

    Popup::DrawSettingsPopup(Translate("ToolBar.Settings"));
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
