#pragma once

#include "data/SosUiData.h"
#include "gui/CharacterEditPanel.h"
#include "gui/OutfitEditPanel.h"
#include "gui/popup/OutfitSelectPopup.h"
#include "i18n/Translator.h"
#include "i18n/translator_manager.h"
#include "service/OutfitService.h"
#include "service/SosDataCoordinator.h"
#include "task.h"

#include <RE/A/Actor.h>
#include <RE/R/Renderer.h>
#include <stdexcept>
#include <windows.h>

namespace SosGui
{

enum class MainMenuAction : std::uint8_t
{
    none,
    create_outfit,
};

class SosGuiWindow
{
    struct InitFail final : std::runtime_error
    {
        explicit InitFail(const char *msg) : std::runtime_error(msg) {}
    };

    using Slot  = RE::BIPED_MODEL::BipedObjectSlot;
    using Armor = RE::TESObjectARMO;

    SosUiData          m_uiData;
    OutfitService      m_outfitService;
    SosDataCoordinator m_dataCoordinator;
    CharacterEditPanel character_edit_panel_;
    OutfitEditPanel    outfit_edit_panel_;
    bool               m_isShowPanels = true;

public:
    SosGuiWindow();

    ~SosGuiWindow();

    static auto Init(HWND hWnd, const RE::BSGraphics::RendererData &renderData) -> void;
    static auto ShutDown() -> void;

    auto Refresh() const -> void;
    auto OnPostDisplay() -> void;

private:
    auto Draw() -> void;
    void MainMenuBar();
    void OnImportSettings();

    static auto EnableQuickSlot(bool enable) -> bool;
};
} // namespace SosGui
