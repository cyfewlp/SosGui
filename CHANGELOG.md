## [unreleased]

### 🚀 Features

- *(Add Armors)* Show a progress bar in popup and show conflict solution for user.
- Add `ArmorEntry` to wrap raw `Armor` pointer. This can be speed up armor sort and compare.
- OutfitList: default focus the input-text item.

### 🐛 Bug Fixes

- Add lucide icon font to imgui;
- Remove `ImportSettings` result verify: no settings also return false;
- *(ArmorView)* Call filter before emplace armor in `reset_view_data`
- *(ArmorView)* Set `armor_count` before filter
- `OutfitList` clear `editing` outfit when no select outfit.
- `OutfitList`: fix the name filter;

### 💼 Other

- Install translate files; remove nerd font file

### 🚜 Refactor

- Remove MultiSelection flags build functions; fix some spell
- Remove all modal popup class
- Use GetPluginInterfaceDir API
- *(edit panel)* Move `playable` and "template armor" checkout to sidebar;
- Remove "remove armors in outfit" feature
- Remove the OutfitList: bad usage, slow
- Remove outfit edit in list context menu: unnecessary
- Migrate to RE Awaitable
- Remove ArmorContainer; remove ArmorGenerator and replaced by ArmorSource + `armor_source_refr_`;
- *(edit panel)* Remove `isTabItemAppear` and replaced by compare old `ArmorSource`; remove `imgui_internal` dependency;
- Replace `ActorPolicyContainer` implement from boost to STL map; fix `ArmorView` slot filter bug.
- Remove boost features; Refactor `ActorOutfitMap` to `ActorOutfitContainer` and implement by STL `vector`;
- Remove `ActorPolicyView` and merged to `CharacterEditPanel`; `CharacterEditPanel` use the `TreeNode` renter actor state and auto-switch ;rename `EagerTask` build to spawn;
- Merge `OutfitListTable` and `OutfitEditPanel`
- Outfit view show name in tab-item title.
- EditPanel: move the `DrawArmorViewFilter` to the `DrawArmorView` inner;
- Remove main view sidebar;
- Move global `translator` to `SosGuiMenu`: initialized before `SosGuiWindow`
- `OutfitList`: remove `SosUiData` and `OutftiService` dependencies and replaced by params;

### ⚡ Performance

- *(outfit list)* Remove `DebounceInput`

### ⚙️ Miscellaneous Tasks

- Remove unused test
- Clang-tidy: disable c-style vararg fun and trailing return-tyle on lambda warnings
- Upgrade imgui to v1.92.7
## [0.1.0] - 2026-03-30

### 🚀 Features

- *(cmake,SosGui)* Add add_build_lucide_icons cmake function to repacke lucide icons from a icon list file;

### 💼 Other

- By Papyrus event call SkyrimOutfitSystemSE functions
- Use TabBar replace RadioButton;
- Fix: move the code that restores outfit armor to view to after the batch add code
- ToolBar add a Views menu to manual control view showing;
- Refactor Popups: popup now only hold on demand;
- Add wrap class RankedArmor to speed up search the armor rank;
- Add ImGuiScop;
- Format
- Format
- Use `consteval` optimize `ImGuiUtil`  flags wrap class
- * Move imgui flags wrap class to common module
- * Disable interactive with armor view when outfit is `Untitled`;
- * Merge Config to UiSetting
- * remove EditPanel PushId*this): cause imgui.ini continuously expanding
- * use icon NF_OCT_DIFF_ADDED
- * update UI layout
- AboutPopup
- * directly compile ImGui
- Add UiSettingsLoader
- Optimize FontManager & UiSettings
- Focus window if window not focused when click the window icon button in the sidebar
- Move `Enable` & `Quickslot` to File menu;
- Directly erase outfit when unmark favorite outfit
- Add new gui CharacterEditPanel to replace SosGuiCharacters &SosGui;
- Refresh icon for RefreshPlayerArmor
- Update assets

### 🚜 Refactor

- Fix all build errors
- *(SosGui)* Remove font manager module; Migrate to common FontManager; Migrated to imgui_manager
- *(SosGui)* Remove About/SettingsPopup; Migrated to lucide icons;
- *(SosGui)* Depecated translator module and Migrate to i18n::Translator(WIP)

### 📚 Documentation

- Add git-cliff and CHANGELOG

### ⚙️ Miscellaneous Tasks

- Split SosGui from JamieMods
