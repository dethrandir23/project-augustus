# AGENTS.md

## Build & Execution
- **Build**: `cmake -B build && cmake --build build`
- **Binary Output**: The executable is located in `build/bin/project-augustus`.
- **Testing**: Uses [Catch2](https://github.com/catchorg/Catch2). Tests are defined in `CMakeLists.txt` and can be run via `ctest --test-dir build` after building.
- **Emscripten**: The project supports WebAssembly builds via `EMSCRIPTEN=1 cmake ...`.

## Architecture
- **Core**: A custom **Entity Component System (ECS)** implementation.
    - `src/Core/ECS/`: Base `Entity` and `Component` classes.
- **UI**: Hybrid architecture.
    - **Native**: Uses **Dear ImGui** with GLFW/OpenGL3 for development/debug tools.
    * `src/App/UI/UIManager.cpp`: Manages windowing and ImGui context.
    - **Web (Planned)**: React/TypeScript frontend communicating via WebSockets/WebView.
- **Data-Driven**: Game world and entities are defined via **JSON**.
    - Static definitions: `data/core/`
    - Runtime saves: **MessagePack** compressed with **LZ4**.
- **Directory Structure**:
    - `src/Core`: ECS core and base types.
    - `src/Game`: High-level gameplay logic.
    - `src/Economy`: Economic systems (Markets, Orderbooks).
    - `src/World`: Map, TradeNodes, and world simulation.
    - `src/App`: Application layer and entry points.

## Development Conventions
- **C++ Standard**: C++20.
- **Dependencies**: Uses `FetchContent` for major dependencies like `webview`, `Catch2`, and `imgui`.
- **Naming**: Follows standard C++ PascalCase for classes and camelCase for methods/variables.
- **Data Flow**: `GameState` $\to$ `JSON` $\to$ `MessagePack` $\to$ `LZ4` $\to$ Save File.

## Economic Fixes (Session 3)
### Mod Data Balance for Alpha
- **`mock/core/data/common/trade_nodes.json`**: Fixed consumption_profile IDs, local_pipelines IDs, upped `initial_capital` to 50000, added `starting_inventory` (200 grain, 100 wood, 100 stone).

- **`mock/core/data/common/pipelines.json`**: Removed `core_water` from basic consumption (item doesn't exist). Added 8 extraction pipelines (wood, stone, clay, sand, coal, iron, sulphur, saltpeter). Added `core_population_demand_024` with demand for intermediate goods (lumber, brick, glass, paper, cloth) plus existing final goods.

- **`mock/core/data/common/factories.json`**: Added 8 extraction factory templates with `build_cost: 3000`.

- **`mock/core/data/scenarios/debug_scenario.json`**: Changed trade node `additional_pipelines` to `core_population_demand_024`, added starting_inventory with grain/wood.

- **`mock/core/data/economy/settings.json`**: Added interest rate and market tax settings.

- **`data/core/`**: Sync'd all changes from mock mod.

### OrderBook Performance
- **`src/Economy/Orderbook.h`**: Replaced `push_back + std::sort` (O(n log n)) with `std::lower_bound` + `insert` (O(n)) for order insertion. Removed unused `BuyOrderComparator`/`SellOrderComparator` structs. Added `#include <algorithm>`.

### Current State (End of Session 3)
- **200-tick test**: 0 failures, save/load OK, balance stable at 5000.
- **AI factories**: 3 built (extraction/low-input preferred).
- **Demand chain now includes intermediate goods** (lumber, paper, brick, glass, cloth) via `core_population_demand_024`.
- **Economy**: Better liquidity with higher trade node capital and broader demand. Next bottleneck is supply-chain depth.
### Problems Fixed
- **Cold-start economy**: AI always built consuming factories (explosives) first, draining balance on unfilled buy orders.
- **Starting inventory sold prematurely**: Companies sold their starting 100 wood/stone to the market, making consuming factories appear viable, then had to buy inputs back.
- **Escrow money wasted**: Buy orders for untraded items locked money forever with no matching sell orders.
- **No production chain bootstrapping**: Companies built consuming factories before extraction factories existed.

### Changes Made
1. **`EconomyEvaluator.cpp`**: `scoreFactoryProfitability()` now checks actual market liquidity (`getBestAsk() > 0`) instead of theoretical producibility. Inputs without sell orders get 5x cost penalty × 2x total cost multiplier. This ensures the AI builds extraction (zero-input) factories first.
2. **`CompanyBrain.cpp` `sellSurplus()`**: Skips selling items that are inputs to any owned factory. Prevents companies from selling their starting inventory needed for production.
3. **`CompanyBrain.cpp` `buyInputs()`**: Uses `getBestAsk()` instead of `getPrice()` to only buy items with actual sell orders. Uses `getBook()->getBestAsk()` price instead of `lastTradedPrice` so buy orders match existing sell orders at the ask price.
4. **`Economy/Orderbook.h`**: Added `#pragma once` header guard (was missing, causing double-include errors).

### Current State
- **200-tick headless test**: 0 failures, save/load OK.
- **Player balance**: Stable at 5000 (no bankruptcy).
- **AI factories**: 3 built (1 per company, all extraction/low-input).
- **Economy**: Stable but not growing — intermediate goods (paper, lumber) lack buyer demand. Trade nodes only consume final goods (tools, clothes, furniture, food). Next step: broaden trade node demand or add market-maker for intermediate goods.
- **Remaining**: Debug `[BUY]` prints in `CompanyBrain.cpp:94` (harmless, shows market activity).
