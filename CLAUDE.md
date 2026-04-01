# CLAUDE.md — Pico Launcher Repository Guide

This document provides context for AI agents working in the `pico-launcher` repository.

## Project Overview

Pico Launcher is a NDS (Nintendo DS) homebrew ROM browser frontend for Pico Loader. It is written in C++ targeting the ARM9 processor and built with the [BlocksDS SDK](https://blocksds.github.io/docs/).

## Build Instructions

```sh
make
```

- Run `make` from the repo root.
- Requires BlocksDS SDK installed at `/opt/blocksds/core`.
- Uses `Makefile` → `Makefile.arm9` / `Makefile.arm7`.
- Output binary: `LAUNCHER.nds`.

## Architecture Overview

```
arm9/source/           — Main application source code (ARM9 processor)
arm7/                  — ARM7 coprocessor code (audio, wireless, RTC)
common/                — Shared code between ARM9 and ARM7
libs/                  — Third-party libraries (FatFS, ArduinoJson, etc.)
_pico/                 — SD card resources (themes, fonts, etc.)
```

### Key ARM9 Source Directories

| Path | Description |
|------|-------------|
| `arm9/source/App.h` / `App.cpp` | Main application class; owns all VRAM, views, controllers, and the main loop |
| `arm9/source/themes/` | Theme system: `ThemeInfo`, `ThemeInfoFactory`, `ThemeFactory`, `ITheme`, material/custom theme implementations |
| `arm9/source/services/settings/` | App settings persistence: `AppSettings`, `IAppSettingsService`, `JsonAppSettingsSerializer` |
| `arm9/source/romBrowser/` | ROM browser: controller, state machine, views, viewModels |
| `arm9/source/romBrowser/views/` | UI views: `DisplaySettingsBottomSheetView`, `BottomSheetView`, etc. |
| `arm9/source/romBrowser/viewModels/` | ViewModels: `DisplaySettingsViewModel`, etc. |
| `arm9/source/gui/` | Low-level GUI framework: `GraphicsContext`, `VramContext`, views, input |
| `arm9/source/fat/` | FatFS wrapper classes: `File`, `Directory` |
| `arm9/source/animation/` | Animation interpolators and curves |

## Key Patterns

### MVVM Pattern
- **Views** consume ViewModels for data and dispatch user actions.
- **ViewModels** interact with Controllers for business logic.
- **Controllers** own services (settings, SD folder, etc.) and implement state transitions.

### State Machine
- `RomBrowserStateMachine` with `RomBrowserState` enum and `RomBrowserStateTrigger` enum.
- State transitions are fired via `_stateMachine.Fire(trigger)`.
- Triggers are handled in `RomBrowserController::HandleTrigger()`.

### Settings Persistence
- `AppSettings` holds all user settings as plain fields.
- Persisted to SD card as JSON via `JsonAppSettingsSerializer`.
- Path: `/_pico/settings.json` (or similar).
- To save: `_appSettingsService->Save()` — typically done via `_saveSettingsPending = true` and flushed in `HideDisplaySettings()`.

### Theme System
- Themes live at `/_pico/themes/<folder>/theme.json` on the SD card.
- `ThemeInfoFactory` reads a theme folder and creates `ThemeInfo` objects.
- `ThemeFactory` instantiates the actual `ITheme` implementation from a `ThemeInfo`.
- `App::LoadTheme()` applies the theme at startup using `AppSettings.theme` (the folder name).
- Changing `AppSettings.theme` and saving will apply the new theme on next launch.

### View Lifecycle
Views implement these methods:
- `InitVram(const VramContext&)` — allocate VRAM resources
- `Update()` — update positions and state
- `Draw(GraphicsContext&)` — render to screen
- `HandleInput(const InputProvider&, FocusManager&)` — process key input
- `MoveFocus(View*, FocusMoveDirection, View*)` — handle focus navigation

### Icon Buttons
- Use `IconButton2DView` with VRAM-loaded tile graphics.
- Icon tiles are compiled-in C arrays (e.g., `hGridIconTiles`, `hGridIconTilesLen`).
- Call `LoadIcon(vramManager, tiles, tilesLen)` to upload and get a VRAM offset.
- Call `SetIconVramOffset(offset)` on the button.

### Labels
- Use `Label2DView` with font from `IFontRepository`.
- `fontRepository->GetFont(FontType::Medium11)` for titles, `FontType::Regular10` for labels.

### Bottom Sheets
- Extend `BottomSheetView` → `DialogView` → `View`.
- Typical row Y positions: Title=16, Row1=46, Row2=78, Row3=112.
- Buttons in a row start at X=70, spaced 32px apart.

### FatFS Directory Enumeration
Use `fat/Directory.h` wrapper:
```cpp
#include "fat/Directory.h"

Directory dir;
if (dir.Open("/_pico/themes/") != FR_OK) return 0;
FILINFO fileInfo;
while (dir.Read(&fileInfo) == FR_OK && fileInfo.fname[0] != 0)
{
    if (fileInfo.fattrib & AM_DIR)
    {
        // process subdirectory: fileInfo.fname
    }
}
```

## Conventions

- **Default branch**: `develop`
- **Language**: C++ with NDS-specific types (`u8`, `u16`, `u32`, `TCHAR`, `vu8`, etc.)
- **Memory**: NDS has limited VRAM — be careful with allocations; avoid heap where possible
- **String type**: `String<CharType, MaxLen>` — fixed-size, stack-allocated; e.g. `String<char, 64>`
- **Header guards**: `#pragma once`
- **Includes in headers**: Prefer forward declarations over includes
- **Thumb mode**: Files needing size optimization use `.thumb.cpp` extension and include `#pragma GCC optimize("Os")` near the top
- **Screen size**: NDS sub-screen is 256×192 pixels; main screen (top) is also 256×192
- **Include paths**: Relative to `arm9/source/` (e.g. `#include "themes/ThemeInfo.h"`)
