//
// Created by jamie on 2025/4/4.
//

#pragma once

#include "SosGui.h"

enum ImGuiKey : int;

namespace SosGui
{

struct GamepadEventInterceptor : RE::MenuEventHandler
{
    bool is_gamepad_event = false;

    auto CanProcess(RE::InputEvent *a_event) -> bool override { return (a_event != nullptr) && a_event->eventType == RE::INPUT_EVENT_TYPE::kButton; }

    auto ProcessButton(RE::ButtonEvent *a_event) -> bool override
    {
        is_gamepad_event = a_event->device == RE::INPUT_DEVICE::kGamepad;
        return false;
    }
};

struct MenuToggleKeyboardEventHandler : RE::MenuEventHandler
{
    auto CanProcess(RE::InputEvent *a_event) -> bool override;

    auto ProcessButton(RE::ButtonEvent *a_event) -> bool override { return false; }
};

class SosGuiMenu : public RE::IMenu
{
    bool                          m_fShow = false;
    i18n::Translator              translator_;
    std::unique_ptr<SosGuiWindow> m_sosGui = nullptr;
    GamepadEventInterceptor       gamepad_event_interceptor_;

public:
    static constexpr std::string_view MENU_NAME = "SosGuiMenu";

    static void RegisterMenu();

    static void ShutDown() { SosGuiWindow::ShutDown(); }

    void PostDisplay() override;

    void ToggleShow();

    constexpr auto IsShowing() const -> bool { return m_fShow; }

    auto ProcessMessage(RE::UIMessage &a_message) -> RE::UI_MESSAGE_RESULTS override;

private:
    void process_scaleform_event(const RE::BSUIScaleformData *data);

    void OnShow();
    void OnHide();
    void OnKeyEvent(RE::GFxEvent *event, const bool down) const;
};
} // namespace SosGui
