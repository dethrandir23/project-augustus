# Project Augustus — Headless Test Report & Analysis

Generated: 2026-05-23 | 200 ticks | 1 player + 3 AI companies

---

## Test Results Summary

| Metric | Value |
|--------|-------|
| Ticks succeeded | 200 / 200 |
| Ticks failed | 0 |
| Save/Load | OK (verified) |
| Market query | OK (1 market) |
| Avg tick time | 1.56 ms |
| Player balance | 5000 (spent 5000 on factory) |
| Player factories | 1 |
| AI factories built | 2 |
| Total companies | 5 (1 scenario + 1 player + 3 AI) |
| Pending events | 0 |
| Exceptions | 0 |

---

## Issues Fixed During This Work

### 1. Fixed: `getPlayerState()` nested JSON bug
**File:** `src/Api/EngineController.cpp:152`

`getPlayerState()` was assigning `j["components"] = player->ToJson()`, which itself contains `id`, `name`, `type`, and nested `components`. This created a double-nested structure (`components.components.WalletComponent`), breaking all state queries from the frontend.

**Fix:** Serialize only the component map directly:
```cpp
j["components"] = nlohmann::json::object();
for (const auto& [key, comp] : player->GetAllComponents()) {
    j["components"][key] = comp->ToJson();
}
```

### 2. Fixed: `SaveUtils::loadGame()` and `listSaves()` wrong config key
**File:** `src/IO/SaveUtils.cpp:61,95`

`saveGame()` correctly used `"storage.save_path"` as the TOML config key, but `loadGame()` and `listSaves()` used `"save_path"` (missing the `storage.` prefix). This caused load operations to always fail returning `"no_save_path"`.

**Fix:** Changed both to use `"storage.save_path"` to match `saveGame()`.

### 3. Fixed: `SaveUtils::loadGame()` missing `.save` extension
**File:** `src/IO/SaveUtils.cpp:70`

`saveGame()` writes files with `name + ".save"` extension, but `loadGame()` constructed the path without the extension, causing file-not-found errors.

**Fix:** Appended `.save` in the load path construction.

### 4. Fixed: Economy evaluator relies on market prices that are initially 0
**File:** `src/AI/Evaluators/EconomyEvaluator.cpp:20`

`scoreFactoryProfitability()` used `MarketComponent::getPrice()` which returns `lastTradedPrice` (initialized to 0.0). With no trades occurring, all prices were 0, making every factory have negative profitability scores (revenue=0, cost>0). This prevented the AI from ever building factories.

**Fix:** Added fallback to `ItemManager::items[itemId].base_price` when the market price is 0 or negative.

---

## AI System Refactoring

### New Architecture

```
AIBrain (abstract base)
├── CompanyBrain (configurable factory-builder AI)
└── TradeNodeBrain (configurable supply/demand AI)

AIManager (orchestrator)
  └── processAll(Gamestate)
        → For each entity with AIControllerComponent:
          → entity->getBrain()->execute(entity, gamestate)

AIControllerComponent (enhanced)
  ├── owns unique_ptr<AIBrain>
  ├── riskTolerance (0-1)
  ├── aggression (0-1)
  └── expansionism (0-1)
```

### File Changes

| File | Action | Purpose |
|------|--------|---------|
| `src/AI/AIBrain.h` | **NEW** | Abstract base class for all AI brains |
| `src/AI/AIBrain.cpp` | **NEW** | Virtual destructor + JSON helpers |
| `src/AI/CompanyBrain.h` | **NEW** | Company AI with configurable parameters |
| `src/AI/CompanyBrain.cpp` | **NEW** | Implementation (moved from inline header) |
| `src/AI/TradeNodeBrain.h` | **NEW** | Trade node AI with configurable parameters |
| `src/AI/TradeNodeBrain.cpp` | **NEW** | Implementation (moved from inline header) |
| `src/AI/AIManager.h` | **MODIFIED** | Implemented orchestrator (was empty stub) |
| `src/AI/AIManager.cpp` | **MODIFIED** | Brain factory registry + processAll loop |
| `src/AI/Components/AIControllerComponent.h` | **MODIFIED** | Added brain ownership + personality traits |
| `src/AI/AIRegistry.h` | **KEPT** | Deprecated in favor of AIManager |
| `src/AI/AIRegistry.cpp` | **MODIFIED** | Removed inline logic includes |
| `src/AI/Logic/CompanyLogic.h` | **KEPT** | No longer used by engine |
| `src/AI/Logic/TradeNodeLogic.h` | **KEPT** | No longer used by engine |
| `src/Game/GameManager.cpp` | **MODIFIED** | processAI uses AIManager instead of AIRegistry |
| `src/Api/EngineController.cpp` | **MODIFIED** | AI companies get CompanyBrain via AIControllerComponent |

### Key Improvements

1. **Proper class hierarchy** — `AIBrain` base class with virtual methods makes adding new brain types trivial. Create a new class extending `AIBrain` and register it in `AIManager::init()`.

2. **Configurable parameters** — Each brain instance has its own thresholds (`sellThreshold`, `investThreshold`, `pickerTemperature`, etc.), stored and serializable. No more hardcoded constants.

3. **Seeded RNG** — `CompanyBrain` uses `std::mt19937` with a configurable seed, replacing `rand()` for deterministic/reproducible AI behavior.

4. **Brain-per-entity** — Each AI entity gets its own brain instance with its own configuration. Personality traits on `AIControllerComponent` (`riskTolerance`, `aggression`, `expansionism`) can influence future decision-making.

5. **Extensible** — To add a new AI behavior:
   ```cpp
   class MyCustomBrain : public AIBrain {
       void execute(Entity& e, Gamestate& gs) override { /* ... */ }
   };
   // Register:
   AIManager::registerBrainType("my_entity_type",
       []() { return std::make_unique<MyCustomBrain>(); });
   ```

6. **Better market selection** — `CompanyBrain::sellSurplus` now finds the market with the best average prices instead of always using the first market entity.

---

## Remaining Issues & What Needs Change

### Critical Issues

#### 1. No economic activity after tick 1
**Observation:** Player balance goes from 10000 → 5000 at tick 1 (factory cost), then stays at exactly 5000 for all 200 ticks. No income, no expenses, no production output sold, no market activity.

**Root cause (likely):** The `sellSurplus` threshold (`sellThreshold = 500.0`) is never reached. Factory production quantities might be very small per tick, and since nothing is sold, nothing accumulates trading volume. Markets remain empty, prices stay at base_price (never updated), creating a deadlock.

**Suggested fix:** Investigate `EconomyUtils::produce()` and `EconomyUtils::executeProduction()` — factory output quantities may be too low to exceed the sell threshold within 200 ticks. Reduce sell threshold or increase production rates for playable pacing.

#### 2. No events triggered
**Observation:** `pending_events = 0` throughout the entire run. No events ever fire.

**Root cause:** The `EventHandler::tickEvents()` runs every tick, but events have trigger conditions (`TURN_GREATER_THAN`, `CHANCE`, etc.) that may never be met with 1 market and no significant economic state changes. Or the event system requires specific population/economic thresholds.

**Suggested fix:** Add diagnostic logging to `tickEvents()` to check if any events match trigger conditions during the run. Consider adding a guaranteed "tutorial" or "first_event" that fires at a specific turn.

#### 3. AI builds factories but never sells or buys
**Observation:** AI builds 2 factories (total) but balance never changes for any company. No market orders are placed.

**Root cause** The `sellSurplus` method requires inventory > `sellThreshold` (500 units). With production rates and only 200 ticks, factories might not produce enough to exceed this threshold. Similarly, the `buildFactories` runs before `sellSurplus` in execution order and spends all available budget, leaving no working capital.

**Suggested fix:** Reduce `sellThreshold` for early-game or implement a "desperation sell" at a lower threshold. Consider ordering sell operations before buy operations.

### Moderate Issues

#### 4. Player company has no AI
The player company (`isAI = false`) never takes any action — no factory building, no trading. In a headless test, this is expected but the auto-generated scenario company also appears inactive. The game economy may need a "passive income" system or automatic workforce mechanics.

#### 5. `createCompany` vs scenario company duplicates
The scenario definition creates a company ("Small Company"), then `setPlayer()` creates additional companies. Companies created by `setPlayer()` use `setPlayerCompanyId()` which overwrites for each call, meaning the last-created company is treated as "player". Design intention is unclear — is the scenario company meant to be the sole player/AI entity, or are `setPlayer` companies additional?

#### 6. Config.toml path hardcoded
`SaveUtils.cpp` hardcodes `"Config.toml"` as a relative path from cwd. This fails when running from a different directory. Should be configurable via CMake define or environment variable.

### Minor Issues

#### 7. `categories.json` has invalid JSON
`data/core/data/common/categories.json` contains a syntax error (stray `x` on line 76) and has no `registry_type`. It's silently skipped by the data loader, but should be fixed or removed.

#### 8. `listSaves()` returns `.save` extension
Save files include the `.save` extension in the return value but `loadGame()` expects the extension to be absent (or present depending on convention). The extension handling should be consistent across all three functions.

#### 9. `loadGame()` receives the extension inconsistently
`loadGame()` now appends `.save` automatically, but `listSaves()` also returns names with `.save`. If the UI passes the listSaves result directly to loadGame, the extension gets doubled (`name.save.save`). The convention needs to be clarified.

**Suggested fix:** Make `listSaves()` strip the `.save` extension, and have `saveGame()` / `loadGame()` always append it internally. This is the most common pattern.

---

## How to Run

### Static Library
```bash
cmake -B build -DBUILD_AS_STATIC_LIB=ON
cmake --build build
# Produces: build/lib/libaugustus_engine.a (37 MB)
```

### Headless Test
```bash
cmake -B build
cmake --build build --target headless_test
./build/bin/headless_test [ticks] [ai_companies] [data_path]
# Examples:
./build/bin/headless_test 200 3 /path/to/data/core/data
./build/bin/headless_test 500 5 data/core/data
```

The test generates `headless_test_report.json` with full per-tick snapshots and timing.
