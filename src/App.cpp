#include "App.h"

#include "PapyrusFunctions.h"
#include "SosGuiMenu.h"
#include "common.h"
#include "log.h"
#include "path_utils.h"

#include <exception>
#include <memory>
#include <string>

namespace SksePlugin
{

namespace
{

/// Registers/unregisters the shortcut handler based on whether the player is in-world.
/// BGSActorCellEvent covers load game, new game, and cell transitions uniformly,
/// unlike SKSE messaging flags (kPostLoadGame / kNewGame) which require separate handling.
struct ActorCellEventSink : RE::BSTEventSink<RE::BGSActorCellEvent>
{
    auto ProcessEvent(const RE::BGSActorCellEvent *a_event, RE::BSTEventSource<RE::BGSActorCellEvent> * /*a_eventSource*/)
        -> RE::BSEventNotifyControl override
    {
        static SosGui::MenuToggleKeyboardEventHandler menu_toggle_keyboard_event_handler;
        if (a_event != nullptr)
        {
            if (a_event->flags == RE::BGSActorCellEvent::CellFlag::kEnter)
            {
                logger::info("Register SosGuiMenu toggle event handler");
                RE::MenuControls::GetSingleton()->AddHandler(&menu_toggle_keyboard_event_handler);
            }
            else
            {
                logger::info("Unregister SosGuiMenu toggle event handler");
                RE::MenuControls::GetSingleton()->RemoveHandler(&menu_toggle_keyboard_event_handler);
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};
} // namespace

auto Initialize() -> bool
{
    using Message = SKSE::MessagingInterface::Message;
    SKSE::GetMessagingInterface()->RegisterListener([](Message *message) {
        if (message->type == SKSE::MessagingInterface::kPostLoadGame)
        {
            logger::debug("Send Event with Init");
        }
        static ActorCellEventSink actor_cell_event_sink;
        if (message->type == SKSE::MessagingInterface::kDataLoaded)
        {
            logger::info("Register ActorCellEventSink.");
            RE::PlayerCharacter::GetSingleton()->AsBGSActorCellEventSource()->AddEventSink<RE::BGSActorCellEvent>(&actor_cell_event_sink);
        }
    });
    SKSE::GetPapyrusInterface()->Register(SosGui::PapyrusFunctions::Register);

    SosGui::App::GetInstance().Init();
    return true;
}
} // namespace SksePlugin

namespace SosGui
{

auto App::Init() -> void
{
    logger::info("Initializing....");

    logger::info("Install D3DInitHook....");
    D3DInitHook = std::make_unique<Hooks::D3DInitHookData>(D3DInit);
}

auto App::MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    switch (msg)
    {
        case WM_DESTROY: {
            SosGuiMenu::ShutDown();
            break;
        }
        default:
            break;
    }
    return RealWndProc(hWnd, msg, wParam, lParam);
}

void App::D3DInit()
{
    try
    {
        GetInstance().DoD3DInit();
        return;
    }
    catch (std::exception &error)
    {
        const auto message = std::format("SimpleIME initialize fail: \n {}", error.what());
        logger::error(message.c_str());
    }
    catch (...)
    {
        const auto message = std::string("SimpleIME initialize fail: \n occur unexpected error.");
        logger::error(message.c_str());
    }
    logger::LogStacktrace();
}

void App::DoD3DInit()
{
    D3DInitHook->Original();

    const auto *renderer = RE::BSGraphics::Renderer::GetSingleton();
    if (renderer == nullptr)
    {
        throw InitFail("Cannot find render manager.");
    }

    const auto &renderData = renderer->GetRuntimeData();
    logger::debug("Getting SwapChain...");
    auto *pSwapChain = renderData.renderWindows[0].swapChain;
    if (pSwapChain == nullptr)
    {
        throw InitFail("Cannot find SwapChain.");
    }
    logger::debug("Getting SwapChain desc...");
    REX::W32::DXGI_SWAP_CHAIN_DESC swapChainDesc;
    if (pSwapChain->GetDesc(&swapChainDesc) < 0)
    {
        throw InitFail("IDXGISwapChain::GetDesc failed.");
    }

    m_hWnd = reinterpret_cast<HWND>(swapChainDesc.outputWindow);
    SosGuiWindow::Init(m_hWnd, renderData);
    RealWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(MainWndProc)));
    if (RealWndProc == nullptr)
    {
        throw InitFail("Hook WndProc failed!");
    }
    m_fInitialized.store(true);
    SosGuiMenu::RegisterMenu();
}
} // namespace SosGui
