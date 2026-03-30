## [unreleased]

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

### ⚙️ Miscellaneous Tasks

- Split SosGui from JamieMods
