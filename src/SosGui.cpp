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
SosGuiWindow::SosGuiWindow() : m_outfitService(m_uiData), m_dataCoordinator(m_uiData, m_outfitService), m_outfitEditPanel(m_uiData, m_outfitService)
{
}

SosGuiWindow::~SosGuiWindow()
{
    m_characterEditPanel.Cleanup();
    m_outfitEditPanel.Cleanup();
    m_isShowPanels = true;
    i18n::SetTranslator(nullptr);
}

auto SosGuiWindow::Init(const HWND hWnd, const RE::BSGraphics::RendererData &renderData) -> void
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

auto SosGuiWindow::Refresh() const -> EagerTask
{
    co_await m_dataCoordinator.Refresh();
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
        m_characterEditPanel.Draw(m_uiData, m_dataCoordinator, m_outfitService);
        m_outfitEditPanel.Draw();
    }
    catch (const std::exception &e)
    {
        logger::LogStacktrace();
        ErrorNotifier::GetInstance().Error(e.what());
    }
}

auto SosGuiWindow::DrawSidebar() -> float
{
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos({0, viewport->WorkPos.y});
    ImGui::SetNextWindowSize({0, viewport->WorkSize.y});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {5.0F, 10.0F});

    const float offsetY = viewport->WorkSize.y * 0.25F;

    float width = 0.0;
    if (ImGui::Begin("##MainSidebar", nullptr, ImGuiEx::WindowFlags().AlwaysAutoResize().NoDecoration().NoMove()))
    {
        width = ImGui::GetWindowWidth();
        ImGui::SetCursorPosY(offsetY);
        const auto styleGuard = ImGuiEx::StyleGuard().Style<ImGuiStyleVar_FramePadding>({5.0F, 5.0F}).Color<ImGuiCol_Button>({});
        ImGui::PushFont(nullptr, Settings::UiSettings::GetInstance()->Title3PxSize());
        auto FocusWindowButton = [](std::string_view icon, const char *tooltip, BaseGui &baseGui) {
            auto isClick = ImGuiUtil::IconButton(icon);
            ImGui::SetItemTooltip("%s", tooltip);
            if (isClick)
            {
                if (baseGui.IsFocused())
                {
                    baseGui.ToggleShow();
                }
                else
                {
                    baseGui.Focus();
                }
            }
        };

        FocusWindowButton(ICON_USERS, Translate1("CharacterEditPanel"), m_characterEditPanel);
        FocusWindowButton(ICON_FILE_PLUS_CORNER, Translate1("EditOutfit"), m_outfitEditPanel);
        ImGui::PopFont();
    }
    ImGui::End();
    ImGui::PopStyleVar();
    return width;
}

void SosGuiWindow::MainMenuBar()
{
    if (!ImGui::BeginMainMenuBar())
    {
        return;
    }
    ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, 15);
    const auto styleGuard = ImGuiEx::StyleGuard().Style<ImGuiStyleVar_FramePadding>({3.0F, 3.0F});
    if (ImGui::BeginMenu(Translate1("ToolBar.File")))
    {
        {
            bool fEnabled = m_uiData.IsEnabled();
            if (ImGui::Checkbox(Translate1("Enabled"), &fEnabled))
            {
                spawn([&] { return m_dataCoordinator.RequestEnable(fEnabled); });
            }
        }

        {
            bool quickSlotEnabled = m_uiData.IsQuickSlotEnabled();
            if (ImGui::Checkbox(Translate1("ToolBar.QuickSlots"), &quickSlotEnabled))
            {
                if (EnableQuickSlot(quickSlotEnabled))
                {
                    m_uiData.SetQuickSlotEnabled(quickSlotEnabled);
                }
            }
            ImGui::SetItemTooltip("%s", Translate1("ToolBar.QuickSlotsToolTip"));
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
        ImGuiUtil::OpenPopup(Translate1("ToolBar.Settings"));
    }
    if (ImGui::MenuItem(Translate1("ToolBar.Close")))
    {
        auto *messageQueue = RE::UIMessageQueue::GetSingleton();
        messageQueue->AddMessage("SosGuiMenu", RE::UI_MESSAGE_TYPE::kHide, nullptr);
    }
    if (ImGui::MenuItem(Translate1("About")))
    {
        ImGuiUtil::OpenPopup("A Extra GUI for SkyrimOutfitSystemRE");
    }

    Popup::DrawSettingsPopup(Translate("ToolBar.Settings"));
    Popup::DrawAboutPopup("A Extra GUI for SkyrimOutfitSystemRE");

    ImGui::PopStyleVar();
    ImGui::EndMainMenuBar();
}

void SosGuiWindow::OnImportSettings()
{
    m_characterEditPanel.OnRefresh();
    m_outfitEditPanel.OnRefresh();
}

EagerTask waitImport(const SosDataCoordinator &dataCoordinator)
{
    logger::debug("wait import");
    co_await dataCoordinator.RequestImportSettings();
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
