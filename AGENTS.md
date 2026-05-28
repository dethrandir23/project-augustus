# AGENTS.md

## Build & Execution
- **Native build**: `cmake -B build && cmake --build build` → `build/bin/project-augustus`
- **Run**: `build/bin/project-augustus mock/load_order.json` (loads mock mod at runtime)
- **Static lib only**: `BUILD_AS_STATIC_LIB=ON cmake -B build && cmake --build build`
- **WASM**: `EMSCRIPTEN=1 cmake -B build && cmake --build build`
- **Godot plugin**: `BUILD_GODOT_PLUGIN=ON cmake -B build && cmake --build build` → `addons/project-augustus/`
- **C++ Standard**: C++20

## Testing
- **Framework**: Catch2 v3 (fetched via FetchContent)
- **Run all**: `ctest --test-dir build`
- **Unit tests** (4 files in `tests/`): `build/bin/augustus_tests`
- **Headless integration test**: `build/bin/headless_test <ticks> <companies> <data_root>`
  - Defaults: 200 ticks, 3 AI companies, uses `data/core/data`
  - Writes report to `headless_test_report.json`
  - Defined in CMakeLists.txt with `DATA_ROOT` compile-time define
  - Auto-registered as ctest `headless_test` (100 ticks, 3 companies, `data/core/data`)
- **Config.toml**: Required for save/load tests. Headless test creates a temporary one if missing.
- **Note**: `tests/test_economy.cpp` is currently empty (0 bytes).

## Architecture
- **Custom ECS** in `src/Core/ECS/` (Entity + Component base classes)
- **Engine singleton**: `augustus_engine::EngineController::instance()` in `src/Api/EngineController.h`
- **Data-driven**: Game world defined via JSON files
  - `mock/core/data/` = active mod (loaded at runtime via `mock/load_order.json`)
  - `data/core/` = stale copy, used only by headless test (`DATA_ROOT`)
- **Runtime saves**: MessagePack + LZ4 compression
- **Key directories**:
  - `src/Core/` — ECS base types
  - `src/Game/` — gamestate, managers, event handler
  - `src/Economy/` — markets, orderbooks, company brain
  - `src/World/` — map, trade nodes
  - `src/App/` — entry point (`main.cpp`)
- **Native ImGui UI** (`src/App/UI/`): Deprecated/broken after ECS refactor. Excluded by default in CMakeLists.txt.

## Frontend (Active UI)
- **Stack**: React 19, TypeScript 6, Vite 8, zustand (state), @tanstack/react-query
- **Build**: `npm run build` in `frontend/` → output to `src/frontend/dist/`
- **Lint**: `npm run lint` (eslint)
- **Typecheck**: `tsc -b` (part of `npm run build`)
- **Embedded in native binary**: `main.cpp` reads `src/frontend/dist/index.html`, inlines JS/CSS via `inlineFrontend()`, sets via `webview::set_html()`
- **Bridge format**: Frontend sends `{type, payload}` (stringified). Bridge in `main.cpp` parses and calls C++ API.
- **Key components**: `Layout.tsx`, `MainMenu.tsx`, `ScenarioSelect.tsx`, `Dashboard.tsx`, `MarketView.tsx`, `CompanyView.tsx`, `TradeNodeView.tsx`, `EventView.tsx`, `SaveDialog.tsx`, `LoadGame.tsx`
- **Store**: `frontend/src/store/engineStore.ts` — single zustand store wrapping `EngineBridge`

## Key Gotchas
- Mock mod is the active data source; always edit `mock/core/data/` for balance changes
- `src/App/UI/` (ImGui) is **not compiled** — remove `UI_SOURCES` exclusion from CMakeLists.txt if reviving it
- `WebApi.cpp` is excluded from native builds via CMakeLists.txt (`#ifdef __EMSCRIPTEN__`-guarded in `main.cpp`); native API bindings live in `EngineController.h` + `main.cpp` webview bindings
- Headless test needs `Config.toml` at CWD with `[storage] save_path = "..."` for save/load
- **Never use `std::cout`, `std::cerr`, or `printf` for logging** — always use `Console::log(msg, LogType::INFO/ERROR/WARNING)` from `DevTools/Console.h`. Include via: `#include "DevTools/Console.h"` (relative to `src/`). This ensures output reaches the Godot/webview console.
- `opencode.json`: only `lsp: true` — no custom instructions configured
- No CI workflows present in `.github/`
