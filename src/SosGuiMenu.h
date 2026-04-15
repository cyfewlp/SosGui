//
// Created by jamie on 2025/4/4.
//

#pragma once

#include "SosGui.h"

enum ImGuiKey : int;

namespace SosGui
{

class SosGuiMenu : public RE::IMenu
{
    bool                          m_fShow = false;
    i18n::Translator              translator_;
    std::unique_ptr<SosGuiWindow> m_sosGui = nullptr;

public:
    static constexpr std::string_view MENU_NAME = "SosGuiMenu";

    static void RegisterMenu();

    static void ShutDown() { SosGuiWindow::ShutDown(); }

    void PostDisplay() override;

    void ToggleShow();

    constexpr auto IsShowing() const -> bool { return m_fShow; }

private:
    auto ProcessMessage(RE::UIMessage &a_message) -> RE::UI_MESSAGE_RESULTS override;

    void OnShow();
    void OnHide();
};
} // namespace SosGui
