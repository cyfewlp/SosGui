## [unreleased]

### 🚀 Features

- *(Add Armors)* Show a progress bar in popup and show conflict solution for user.
- Add `ArmorEntry` to wrap raw `Armor` pointer. This can be speed up armor sort and compare.
- OutfitList: default focus the input-text item.
- Integrate tracy profiler; ArmorView: fix a dereference bug and use `erase_if` optimize `prune_view_by_slot_filter`
- `OutfitList`: show name `conflict` error in create popup.
- Add new option to only show no conflict armors;
- Support preview armor
- Preview hovered armor
- Armor Preview: locate by model center
- Outfit List: cache the filtered list item count to make the scrollbar reliable;
- Outfit List: create outfit support shortcut and can trigger from main menu bar;
- Refresh player armors when menu close
- Skip empty slot mask armors(马厩、窑炉);
- Always set selected slot from `slot_mast` when the `show_no_conflict_armors_` checked
- CharacterEditPanel: support filter outfit in the outfit combo
- Optimize the add actor combo preview value.
- Set window default pos/size
- Character/OutfitEdit Panel: near actor/objects sorted by formid.
- Add custom ImGui multi-selection`ApplyRequest` implementtion to support descend view data(only change view data index when sort).
- Panel can close and can toggle showing in main menu-bar;
- OutfitList: support create a outfit copy.
- Support close "create outfit" poup by escape
- Support gamepad: disable all key mapping; close menu if ImGui no focused window and received a "Cancel" event.
- OutfitEdit: push clip.Userindex(independent with data) as id seed. This can supoort auto move focus to next row if focused row filtered.
- OutfitEdit: use child window push a focus scope for "main content" instead of group
- Refresh all actors outfit/policy outfit when close menu or trigegr from menu bar
- Disable focus preview armor window on appearing.
- Show conflict armors error when focused "Add" button in armor row.
- Support clear auto-switch policy outfit
- Support clear active outfit

### 🐛 Bug Fixes

- Add lucide icon font to imgui;
- Remove `ImportSettings` result verify: no settings also return false;
- *(ArmorView)* Call filter before emplace armor in `reset_view_data`
- *(ArmorView)* Set `armor_count` before filter
- `OutfitList` clear `editing` outfit when no select outfit.
- `OutfitList`: fix the name filter;
- Fix `Untitled` translation
- Remove `ImGuiListClipper`: avoid combine filter and clipper(not evenly item count); Each row has low draw cost and ·ItemAdd· also perform a clipping.
- OutfitContainer: sort outfits after batch add and `add` method now add outfit by sorted pos;  SosNativeCaller/OutfitService: adjust all rvalue param;
- OutfitContainer/ActorOutfitContainer: add the `lower_bound` function to differentiate `find`. Add `try_emplace` fun and it behavior similar stl container;
- OutfitEdit: fix selected armor slot mark logic; ui: improve conflict armors highlight style;
- Armor Preview: add a offset with world space to armor local position;
- Fix coroutine hang string reference bug.
- Set update flag to refresh model
- EditingOutfit: set the `invalid` field for assign value; `SlotPolicy` help popup closeable.
- SosGUiMenu: use menu context
- About popup can close
- CharacterEditPanel.cpp: fix title translate.
- OutfitEditPanel: call `OutfitListTable::on_refresh` in `on_refresh`; refactor the: "view_item_count_", pass `INT_MAX` to imgui list clipper and seek item by `view_item_count_` and re-calculate when `UserIndex` not equal `view_item_count_`
- OutfitEdit: set slot filter to `Pass_Has_Any_Slot` when check any slot.
- OutfitList: set `editing` and `editing_id` to INVALID after change.

### 💼 Other

- Install translate files; remove nerd font file
- Armor Preview: relayout on window size change; fix: fix the preview armor no update bug

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
- Outfit: now `slot-policies` only stored in `EditingOutfit` and update when select a outfit
- Outfit: store `slot-policy` as enum value to reduce memory usage by 40 times
- Rename `Policy` to `AutoSwitch`
- Merge `ActorPolicyContainer` to `ActorOutfitContainer`; fix the hanging reference of editing outfit;
- Remove `may_update_table_sort_dir` and handle sort explicitly.
- `SosUiData`: `SosUiData` now is a POD; remove the `actors` vector. The same feature should access `actor_outfit_container`
- Re-introduce list clipper; optimize code
- OutfitEdit/ArmorView: use `ArmorView::filter` + `ImGuiListClipper` instead `reset_view_data` after any fliter, outfit changes.
- Remove class `MultiSection`
- Remove `BaseGui`, `Cleanable`
- Outfit Edit: remove `uiData`,replace ref member `OutfitService` to ptr; Add a `invalid` flag to `EditingOutfit`, this can reduce search times.
- Cleanup `UiSettings`
- Remove unused settings
- Update README; remove `InvisibleButton` for escape to quit window: already support by the `ConfigNavEscapeClearFocusWindow` flag.

### 📚 Documentation

- Update CHANGELOG; remove unused code;

### ⚡ Performance

- *(outfit list)* Remove `DebounceInput`

### 🖼️ UI Changes

- OutfitEdit: adjust filters layout: use   `CollapsingHeader` instead of `SeparatorText`

### ⚙️ Miscellaneous Tasks

- Remove unused test
- Clang-tidy: disable c-style vararg fun and trailing return-tyle on lambda warnings
- Upgrade imgui to v1.92.7
- Stuff
- Some adjust
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
