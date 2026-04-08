//
// Created by jamie on 2025/5/25.
//

#include "data/ActorOutfitContainer.h"
#include "gui/BaseGui.h"
#include "util/ImGuiUtil.h"

namespace SosGui
{

struct SosUiData;
class SosDataCoordinator;
class OutfitService;

class CharacterEditPanel final : public BaseGui
{
public:
    void Focus() override;

    void Cleanup() override {}

    void Draw(SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService);

private:
    void DrawCharactersPanel(SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService);
    void DrawCharactersInfo(SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService);
    void DrawOutfitsCombo(SosUiData &uiData, const OutfitService &outfitService, RE::Actor *actor);
    auto get_outfit_display_name(const RE::Actor *currentActor, AutoSwitch policy, SosUiData &uiData) -> std::string_view;
    void draw_auto_switch(RE::Actor *currentActor, SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService);

    ImGuiUtil::DebounceInput debounce_input_;
    const RE::Actor         *outfit_popup_target_actor_{nullptr};
    AutoSwitch               selected_policy_ = AutoSwitch::None;
};

} // namespace SosGui
