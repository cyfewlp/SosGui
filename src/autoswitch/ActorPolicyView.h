//
// Created by jamie on 2025/5/14.
//

#pragma once

#include "gui/popup/OutfitSelectPopup.h"

#include <limits>
#include <memory>

namespace SosGui
{
class SosDataCoordinator;
class OutfitService;
class SosUiData;

namespace AutoSwitch
{
class ActorPolicyView
{
    struct Popup : OutfitSelectPopup
    {
        const RE::Actor *actor          = nullptr;
        uint32_t         selectPolicyId = std::numeric_limits<uint32_t>::max();

        Popup(const RE::Actor *const actor, const uint32_t selectPolicyId) : actor(actor), selectPolicyId(selectPolicyId) {}
    };

public:
    void Draw(const RE::Actor *currentActor, SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService);

private:
    static void Column1Outfit(RE::FormID actorId, uint32_t policyId, SosUiData &uiData);

    std::unique_ptr<Popup> outfitSelectPopup = nullptr;
};
} // namespace AutoSwitch
} // namespace SosGui
