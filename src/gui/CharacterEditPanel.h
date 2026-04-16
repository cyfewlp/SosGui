//
// Created by jamie on 2025/5/25.
//

#include "data/ActorOutfitContainer.h"
#include "data/OutfitContainer.h"
#include "util/ImGuiUtil.h"

namespace SosGui
{

struct SosUiData;
class SosDataCoordinator;
class OutfitService;

class CharacterEditPanel final
{
public:
    void Draw(SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService);

    void toggle_showing() { showing_ = !showing_; }

private:
    void DrawCharactersPanel(SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService);
    void DrawCharactersInfo(SosUiData &ui_data, const SosDataCoordinator &data_coordinator, const OutfitService &outfit_service);
    void draw_auto_switch(
        const ActorOutfitContainer::Entry &editing_actor, const OutfitContainer &outfit_container, const SosDataCoordinator &data_coordinator,
        const OutfitService &outfit_service
    );

    ImGuiTextFilter outfit_name_filter_;
    bool            showing_ = true;
};

} // namespace SosGui
