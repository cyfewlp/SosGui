//
// Created by jamie on 2025/4/4.
//

#include "SosGuiMenu.h"

#include "SosGui.h"
#include "imgui.h"
#include "log.h"
#include "tracy/Tracy.hpp"
#include "util/utils.h"

#include <path_utils.h>
#include <unordered_map>

namespace RE
{
class GFxCharEvent : public RE::GFxEvent
{
public:
    GFxCharEvent() = default;

    explicit GFxCharEvent(UINT32 a_wcharCode, UINT8 a_keyboardIndex = 0)
        : GFxEvent(EventType::kCharEvent), wcharCode(a_wcharCode), keyboardIndex(a_keyboardIndex)
    {
    }

    // @members
    std::uint32_t wcharCode{};     // 04
    std::uint32_t keyboardIndex{}; // 08
};

static_assert(sizeof(GFxCharEvent) == 0x0C);
} // namespace RE

namespace SosGui
{
namespace
{
ImGuiMouseSource ImGui_ImplWin32_GetMouseSourceFromMessageExtraInfo()
{
    LPARAM extra_info = ::GetMessageExtraInfo();
    if ((extra_info & 0xFFFFFF80) == 0xFF515700) return ImGuiMouseSource_Pen;
    if ((extra_info & 0xFFFFFF80) == 0xFF515780) return ImGuiMouseSource_TouchScreen;
    return ImGuiMouseSource_Mouse;
}

auto Creator() -> RE::IMenu *
{
    using Flags = RE::UI_MENU_FLAGS;
    auto *menu  = new SosGuiMenu();
    menu->menuFlags.set(Flags::kPausesGame, Flags::kDisablePauseMenu);
    menu->menuFlags.set(Flags::kUpdateUsesCursor, Flags::kUsesCursor);
    menu->menuFlags.set(Flags::kCustomRendering, Flags::kAlwaysOpen, Flags::kTopmostRenderedMenu);
    return menu;
}

auto GFxKeyToImGuiKey(RE::GFxKey::Code keyCode) -> ImGuiKey;

void OnMouseEvent(RE::GFxEvent *event, const bool down)
{
    const auto &mouseSource = ImGui_ImplWin32_GetMouseSourceFromMessageExtraInfo();
    const auto *mouseEvent  = reinterpret_cast<RE::GFxMouseEvent *>(event);
    auto       &io          = ImGui::GetIO();
    io.AddMouseSourceEvent(mouseSource);
    io.AddMouseButtonEvent(static_cast<int>(mouseEvent->button), down);
}

void OnMouseWheelEvent(RE::GFxEvent *event)
{
    const auto *mouseEvent = reinterpret_cast<RE::GFxMouseEvent *>(event);
    ImGui::GetIO().AddMouseWheelEvent(0, mouseEvent->scrollDelta);
}

void OnKeyEvent(const RE::GFxKey::Code keycode, const bool down)
{
    auto imguiKey = GFxKeyToImGuiKey(keycode);
    ImGui::GetIO().AddKeyEvent(imguiKey, down);
}

void OnKeyEvent(RE::GFxEvent *event, const bool down)
{
    const auto keyEvent = reinterpret_cast<RE::GFxKeyEvent *>(event);
    OnKeyEvent(keyEvent->keyCode, down);
}

void OnCharEvent(RE::GFxEvent *event)
{
    const auto charEvent = reinterpret_cast<RE::GFxCharEvent *>(event);
    ImGui::GetIO().AddInputCharacter(charEvent->wcharCode);
}

void on_user_events(const RE::BSFixedString &event_name)
{
    const auto &user_events = RE::UserEvents::GetSingleton();
    ImGuiKey    imgui_key   = ImGuiKey_None;
    if (event_name == user_events->cancel)
    auto       &io          = ImGui::GetIO();

    if (!io.WantCaptureKeyboard && event_name == user_events->cancel)
    {
        auto *messageQueue = RE::UIMessageQueue::GetSingleton();
        messageQueue->AddMessage(SosGuiMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
    }
    else if (event_name == user_events->up)
    {
        imgui_key = ImGuiKey_UpArrow;
    }
    else if (event_name == user_events->down)
    {
        imgui_key = ImGuiKey_DownArrow;
    }
    else if (event_name == user_events->left)
    {
        imgui_key = ImGuiKey_LeftArrow;
    }
    else if (event_name == user_events->right)
    {
        imgui_key = ImGuiKey_RightArrow;
    }
    if (imgui_key != ImGuiKey_None)
    {
        io.AddKeyEvent(imgui_key, true);
        io.AddKeyEvent(imgui_key, false);
    }
}

void ProcessScaleformEvent(const RE::BSUIScaleformData *data)
{
    switch (const auto &fxEvent = data->scaleformEvent; fxEvent->type.get())
    {
        case RE::GFxEvent::EventType::kMouseDown:
            OnMouseEvent(fxEvent, true);
            break;
        case RE::GFxEvent::EventType::kMouseUp:
            OnMouseEvent(fxEvent, false);
            break;
        case RE::GFxEvent::EventType::kMouseWheel:
            OnMouseWheelEvent(fxEvent);
            break;
        case RE::GFxEvent::EventType::kKeyDown:
            OnKeyEvent(fxEvent, true);
            break;
        case RE::GFxEvent::EventType::kKeyUp:
            OnKeyEvent(fxEvent, false);
            break;
        case RE::GFxEvent::EventType::kCharEvent:
            OnCharEvent(fxEvent);
            break;
        default:
            break;
    }
}
} // namespace

void SosGuiMenu::RegisterMenu()
{
    if (auto *ui = RE::UI::GetSingleton(); ui != nullptr)
    {
        ui->Register(MENU_NAME, Creator);
    }
}

void SosGuiMenu::PostDisplay()
{
    ZoneScopedN(__FUNCTION__);
    m_sosGui->OnPostDisplay();
    FrameMark;
}

void SosGuiMenu::ToggleShow()
{
    m_fShow            = !m_fShow;
    auto  type         = m_fShow ? RE::UI_MESSAGE_TYPE::kShow : RE::UI_MESSAGE_TYPE::kHide;
    auto *messageQueue = RE::UIMessageQueue::GetSingleton();
    messageQueue->AddMessage(MENU_NAME, type, nullptr);
}

auto SosGuiMenu::ProcessMessage(RE::UIMessage &a_message) -> RE::UI_MESSAGE_RESULTS
{
    switch (a_message.type.get())
    {
        case RE::UI_MESSAGE_TYPE::kShow: {
            OnShow();
            break;
        }
        case RE::UI_MESSAGE_TYPE::kHide: {
            OnHide();
            break;
        }
        case RE::UI_MESSAGE_TYPE::kUserEvent: {
            const auto &data = reinterpret_cast<RE::BSUIMessageData *>(a_message.data);
            on_user_events(data->fixedStr);
            break;
        }
        case RE::UI_MESSAGE_TYPE::kScaleformEvent: // never send because we use ImGui
        {
            auto *scaleformData = reinterpret_cast<RE::BSUIScaleformData *>(a_message.data);
            if (scaleformData != nullptr && scaleformData->scaleformEvent != nullptr)
            {
                ProcessScaleformEvent(scaleformData);
            }
            break;
        }
        default:;
    }
    return RE::UI_MESSAGE_RESULTS::kHandled;
}

void SosGuiMenu::OnShow()
{
    m_fShow = true;
    logger::debug("SosGuiMenu::kShow");
    const auto &control_map = RE::ControlMap::GetSingleton();
    control_map->ToggleControls(RE::UserEvents::USER_EVENT_FLAG::kAll, false, false);
    RE::Inventory3DManager::GetSingleton()->Begin3D(RE::INTERFACE_LIGHT_SCHEME::kInventory);

    i18n::SetTranslator(&translator_);
    i18n::UpdateTranslator("english", "english", utils::GetPluginInterfaceDir());
    m_sosGui = std::make_unique<SosGuiWindow>();
    m_sosGui->Refresh();
}

void SosGuiMenu::OnHide()
{
    m_fShow = false;
    m_sosGui.reset();
    const auto &control_map = RE::ControlMap::GetSingleton();
    control_map->ToggleControls(RE::UserEvents::USER_EVENT_FLAG::kAll, true, false);
    i18n::SetTranslator(nullptr);
    if (auto *inventory_manager = RE::Inventory3DManager::GetSingleton(); inventory_manager != nullptr)
    {
        inventory_manager->UnloadInventoryItem();
        inventory_manager->End3D();
    }
    util::RefreshActorArmor(RE::PlayerCharacter::GetSingleton());

    logger::debug("SosGuiMenu::kHide");
    ImGui::GetIO().ClearInputKeys();
}

namespace
{

struct KeyMapEntry
{
    RE::GFxKey::Code gfx_key;
    ImGuiKey         imgui_key;
};

static std::vector<KeyMapEntry> key_map = {
    {RE::GFxKey::kBackspace,    ImGuiKey_Backspace     },
    // {RE::GFxKey::kTab,          ImGuiKey_Tab           },
    // {RE::GFxKey::kClear, ImGuiKey_Clear},
    {RE::GFxKey::kReturn,       ImGuiKey_Enter         },
    {RE::GFxKey::kShift,        ImGuiMod_Shift         },
    {RE::GFxKey::kControl,      ImGuiMod_Ctrl          },
    {RE::GFxKey::kAlt,          ImGuiMod_Alt           },
    {RE::GFxKey::kPause,        ImGuiKey_Pause         },
    {RE::GFxKey::kCapsLock,     ImGuiKey_CapsLock      },
    {RE::GFxKey::kEscape,       ImGuiKey_Escape        },
    {RE::GFxKey::kSpace,        ImGuiKey_Space         },
    {RE::GFxKey::kPageUp,       ImGuiKey_PageUp        },
    {RE::GFxKey::kPageDown,     ImGuiKey_PageDown      },
    {RE::GFxKey::kEnd,          ImGuiKey_End           },
    {RE::GFxKey::kHome,         ImGuiKey_Home          },
    {RE::GFxKey::kLeft,         ImGuiKey_LeftArrow     },
    {RE::GFxKey::kUp,           ImGuiKey_UpArrow       },
    {RE::GFxKey::kRight,        ImGuiKey_RightArrow    },
    {RE::GFxKey::kDown,         ImGuiKey_DownArrow     },
    {RE::GFxKey::kInsert,       ImGuiKey_Insert        },
    {RE::GFxKey::kDelete,       ImGuiKey_Delete        },
    // {RE::GFxKey::kHelp, ImGuiKey_Hel}p
    {RE::GFxKey::kKP_Multiply,  ImGuiKey_KeypadMultiply},
    {RE::GFxKey::kKP_Add,       ImGuiKey_KeypadAdd     },
    {RE::GFxKey::kKP_Enter,     ImGuiKey_KeypadEnter   },
    {RE::GFxKey::kKP_Subtract,  ImGuiKey_KeypadSubtract},
    {RE::GFxKey::kKP_Decimal,   ImGuiKey_KeypadDecimal },
    {RE::GFxKey::kKP_Divide,    ImGuiKey_KeypadDivide  },
    {RE::GFxKey::kNumLock,      ImGuiKey_NumLock       },
    {RE::GFxKey::kScrollLock,   ImGuiKey_ScrollLock    },
    {RE::GFxKey::kSemicolon,    ImGuiKey_Semicolon     },
    {RE::GFxKey::kEqual,        ImGuiKey_Equal         },
    {RE::GFxKey::kComma,        ImGuiKey_Comma         },
    {RE::GFxKey::kMinus,        ImGuiKey_Minus         },
    {RE::GFxKey::kPeriod,       ImGuiKey_Period        },
    {RE::GFxKey::kSlash,        ImGuiKey_Slash         },
    // {RE::GFxKey::kBar, ImGuiKey_Bar                     },
    {RE::GFxKey::kBracketLeft,  ImGuiKey_LeftBracket   },
    {RE::GFxKey::kBackslash,    ImGuiKey_Backslash     },
    {RE::GFxKey::kBracketRight, ImGuiKey_RightBracket  },
    // RE::GFxKey::kQuote, ImGuiKey_
    // RE::GFxKey::kOEM_AX, ImGuiKey_OEM_AX
    // RE::GFxKey::kOEM_102, ImGuiKey_OEM_102
    // RE::GFxKey::kICO_HELP, ImGuiKey_ICO_HELP
    // RE::GFxKey::kICO_00, ImGuiKey_ICO_00
};

auto GFxKeyToImGuiKey(const RE::GFxKey::Code keyCode) -> ImGuiKey
{
    ImGuiKey imguiKey = ImGuiKey_None;

    if (keyCode >= RE::GFxKey::kA && keyCode <= RE::GFxKey::kZ)
    {
        imguiKey = static_cast<ImGuiKey>(keyCode - RE::GFxKey::kA + ImGuiKey_A);
    }
    else if (keyCode >= RE::GFxKey::kF1 && keyCode <= RE::GFxKey::kF15)
    {
        imguiKey = static_cast<ImGuiKey>(keyCode - RE::GFxKey::kF1 + ImGuiKey_F1);
    }
    else if (keyCode >= RE::GFxKey::kNum0 && keyCode <= RE::GFxKey::kNum9)
    {
        imguiKey = static_cast<ImGuiKey>(keyCode - RE::GFxKey::kNum0 + ImGuiKey_0);
    }
    else if (keyCode >= RE::GFxKey::kKP_0 && keyCode <= RE::GFxKey::kKP_9)
    {
        imguiKey = static_cast<ImGuiKey>(keyCode - RE::GFxKey::kKP_0 + ImGuiKey_Keypad0);
    }
    else
    {
        const auto it = std::ranges::lower_bound(key_map, keyCode, std::less<>(), &KeyMapEntry::gfx_key);
        if (it != key_map.end() && it->gfx_key == keyCode)
        {
            imguiKey = it->imgui_key;
        }
    }
    return imguiKey;
}
} // namespace
} // namespace SosGui
