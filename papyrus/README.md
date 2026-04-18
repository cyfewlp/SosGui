# SosGui — Papyrus Scripts

Papyrus scripting module for the **SosGui** Skyrim SE mod.  
These scripts add a voice spell that opens the SosGui custom menu.

---

## Project Structure

```
skyrimse.ppj                    # Pyro project file (SSE)
SosGui.esp                      # Plugin file
Scripts/                        # Compiled .pex bytecode
Source/Scripts/
    SosGui.psc                  # Quest script — adds spell on game start
    SosGuiNative.psc            # Native binding — calls the C++ menu function
    SosGuiShowEffectScript.psc  # Magic effect — triggers ShowSosGuiMenu()
```

---

## Scripts Overview

| Script | Base | Role |
|---|---|---|
| `SosGui` | `Quest` | On game init, adds the `SosGuiShow` spell to the player if not already present. |
| `SosGuiNative` | *(hidden global)* | Declares the native function `ShowSosGuiMenu()` implemented in C++. |
| `SosGuiShowEffectScript` | `ActiveMagicEffect` | When the `SosGuiShow` spell fires, calls `SosGuiNative.ShowSosGuiMenu()` to open the GUI. |

---

## Development Setup

**Requirements**

- Skyrim Special Edition (with **Creation Kit** installed)
- [Papyrus extension for VS Code](https://marketplace.visualstudio.com/items?itemName=joelday.papyrus-lang-vscode) by Joel Day

**Steps**

1. Install the **Papyrus** VS Code extension.
2. Open the workspace file `SkyrimSE.code-workspace`.
3. Configure the extension:
   - Set `Game Path` to your Skyrim SE installation directory  
     (e.g. `D:\Program Files (x86)\Steam\steamapps\common\Skyrim Special Edition`).
4. To compile, open the Command Palette and run:  
   **Tasks: Run Task → pyro: Compile Project (skyrimse.ppj)**

Compiled `.pex` files are output to the `Scripts/` folder.

---

## Build Configuration (`skyrimse.ppj`)

| Setting | Value |
|---|---|
| Game | `sse` (Skyrim Special Edition) |
| Flags | `TESV_Papyrus_Flags.flg` |
| Output | `Scripts/` |
| Optimize | false |
| Release | false |
| Source | `Source/Scripts/` |