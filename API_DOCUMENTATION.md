# Project Augustus API Documentation

This document describes how to build a frontend (Vulkan, Unreal Engine, Unity, custom React app, etc.) that communicates with the Project Augustus game engine.

---

## 1. Linking the Static Library

Build the engine as a static library:

```bash
cmake -B build -DBUILD_AS_STATIC_LIB=ON
cmake --build build
```

Output: `build/lib/libaugustus_engine.a`

### Minimal C++ Example

```cpp
// main.cpp
#include "Api/NativeApi.h"
#include <iostream>

int main() {
    // 1. Initialize engine
    project_augustus::initEngine();

    // 2. Load game data files (see §2 for format)
    std::vector<std::string> files = { /* JSON content strings */ };
    std::vector<std::string> names  = { /* filenames like "items.json" */ };
    project_augustus::loadGameFiles(files, names);

    // 3. Start a scenario
    project_augustus::startScenario("debug_scenario");

    // 4. Create a player company
    project_augustus::setPlayer("My Company", "core_startup_company_001", false);

    // 5. Run ticks
    for (int i = 0; i < 100; i++)
        project_augustus::step();

    // 6. Read state
    std::cout << project_augustus::getSerializedState() << std::endl;

    return 0;
}
```

Compile and link:

```bash
g++ -std=c++20 main.cpp -I/path/to/project-augustus/src \
    -L/path/to/project-augustus/build/lib -laugustus_engine \
    -lglfw -lGL -luuid -llz4 -o my_frontend
```

Or with CMake:

```cmake
add_executable(my_frontend main.cpp)
target_link_libraries(my_frontend PRIVATE /path/to/libaugustus_engine.a)
target_include_directories(my_frontend PRIVATE /path/to/project-augustus/src)
```

---

## 2. Data Loading (Before Scenario Starts)

Before calling `startScenario()`, you must call `loadGameFiles()` with the contents of all game definition files. The `loadGameFiles` function takes two parallel arrays:

- `contents`: the raw JSON string content of each file
- `names`: the filename (used to determine which handler to call)

### File Name → Handler Mapping

| Name (filename) | Handler | Required |
|---|---|---|
| `items.json` | `ITEM_DEFINITIONS` | Yes |
| `pipelines.json` | `PIPELINE_DEFINITIONS` | Yes |
| `factories.json` | `FACTORY_DEFINITIONS` | Yes |
| `economy_settings.json` | `ECONOMY_DEFINITIONS` | Yes |
| `trade_nodes.json` | `TRADENODE_DEFINITIONS` | Yes |
| `companies.json` | `COMPANY_DEFINITIONS` | Yes |
| `markets.json` | `MARKET_DEFINITIONS` | Yes |
| `map.json` | `MAP_DEFINITION` | Yes |
| `scenarios.json` | `SCENARIO_DEFINITION` | Yes |
| `events.json` | `EVENT_DEFINITIONS` | Optional |

If using the built-in `ModLoader`, it reads a `load_order.json` which lists the files and their paths, then calls `loadGameFiles` automatically. For external frontends, replicate this logic (read JSON files → pass to `loadGameFiles`).

---

## 3. Full Native API Reference

All functions are in namespace `project_augustus`, declared in `src/Api/NativeApi.h`.

### 3.1 Initialization

| Function | Description |
|---|---|
| `void initEngine()` | Must be called once before any other API function. Initializes subsystems (InputHandler, AIManager). |

### 3.2 Data Loading & Scenario

| Function | Description |
|---|---|
| `bool loadGameFiles(contents, names)` | Loads game definition files. Returns true if all files loaded successfully. |
| `bool startScenario(scenarioId)` | Starts a scenario by ID (must match a scenario in scenarios.json). Returns true on success. |
| `void setPlayer(name, templateId, isAI)` | Creates a player company. `templateId` must match a company template in companies.json. If `isAI=true`, the AI will control it. |

### 3.3 Game Loop

| Function | Description |
|---|---|
| `void step()` | Advances the game by 1 tick. All AI decisions, market matching, production, and consumption happen here. |
| `void stepN(size_t n)` | Advances by `n` ticks. |

### 3.4 State Queries (All Return JSON Strings)

| Function | Return |
|---|---|
| `std::string getSerializedState()` | Full game state as JSON (all entities, their components, current turn). |
| `std::string getPlayerState()` | The player company entity as JSON (ID, name, type, all component data). |
| `std::string getMarketData(marketId)` | Returns market data. Currently returns ALL markets (ignores `marketId`). |
| `std::string getFactoryStatus(factoryId)` | Returns a specific factory entity as JSON. `factoryId` is a UUID string. |
| `std::string getPendingEvents()` | Returns pending events array as JSON. |

### 3.5 Player Input

| Function | Description |
|---|---|
| `bool sendInput(inputJson)` | Sends a player action. See §4 for available actions and payloads. |

### 3.6 Console / Logging

| Function | Description |
|---|---|
| `std::vector<std::string> readConsole()` | Returns all buffered log messages and clears the buffer. |
| `void logToConsole(msg, type)` | Writes a custom log message. `type` is `LogType` enum: `INFO`, `ERROR`, `WARNING`. |

### 3.7 Save / Load

| Function | Description |
|---|---|
| `bool saveGame(name)` | Saves the current game state as MessagePack + LZ4 compressed to `saves/<name>.save`. |
| `bool loadGame(name)` | Loads a saved game from `saves/<name>.save`. Replaces current game state. |
| `std::vector<std::string> listSaves()` | Returns a list of available save names (without `.save` extension). |

---

## 4. Player Input Actions (`sendInput`)

`sendInput` accepts a JSON string with this structure:

```json
{
    "type": "ACTION_NAME",
    "payload": { ... }
}
```

Optional field in payload for all actions:
- `"companyId": "<uuid>"` — target a specific company. If omitted, targets the player company.

### Available Actions

#### BATCH_EXECUTE
Execute multiple actions in one call.

```json
{
    "type": "BATCH_EXECUTE",
    "payload": [
        { "type": "ADD_MONEY", "payload": { "amount": 1000 } },
        { "type": "STEP_GAME", "payload": { "times": 5 } }
    ]
}
```

#### STEP_GAME
Run game ticks (equivalent to `stepN`).

```json
{
    "type": "STEP_GAME",
    "payload": { "times": 1 }
}
```

#### ADD_MONEY
Add money to a company's wallet.

```json
{
    "type": "ADD_MONEY",
    "payload": { "amount": 5000.0 }
}
```

#### ADD_ITEM
Add items to a company's storage.

```json
{
    "type": "ADD_ITEM",
    "payload": { "itemId": "core_wood", "amount": 100.0 }
}
```

#### SET_PROPERTY
Set a property on a company component.

```json
{
    "type": "SET_PROPERTY",
    "payload": {
        "component": "WalletComponent",
        "property": "balance",
        "value": 50000.0
    }
}
```

Supported components:
| component | property | value type |
|---|---|---|
| `WalletComponent` | `balance` | double |
| `WalletComponent` | `debt` | double |
| `InventoryComponent` | `maxWeight` | double |

#### BUILD_FACTORY
Build a factory for a company.

```json
{
    "type": "BUILD_FACTORY",
    "payload": {
        "templateId": "core_extraction_wood",
        "customName": "My Sawmill"
    }
}
```

`templateId` must match a factory template ID in `factories.json`.

#### SCRAP_FACTORY
Remove a factory from the game.

```json
{
    "type": "SCRAP_FACTORY",
    "payload": { "factoryId": "<uuid>" }
}
```

#### MARKET_BUY_ITEM
Place a buy order on a market.

```json
{
    "type": "MARKET_BUY_ITEM",
    "payload": {
        "marketId": "<uuid>",
        "itemId": "core_wood",
        "price": 15.0,
        "amount": 100.0
    }
}
```

#### MARKET_SELL_ITEM
Place a sell order on a market.

```json
{
    "type": "MARKET_SELL_ITEM",
    "payload": {
        "marketId": "<uuid>",
        "itemId": "core_wood",
        "price": 15.0,
        "amount": 100.0
    }
}
```

#### CANCEL_ORDER
Cancel a pending order and retrieve escrow.

```json
{
    "type": "CANCEL_ORDER",
    "payload": {
        "marketId": "<uuid>",
        "itemId": "core_wood",
        "orderType": "BUY",
        "price": 15.0,
        "amount": 100.0
    }
}
```

#### PAY_DEBT
Pay off the company's debt (if enough balance).

```json
{
    "type": "PAY_DEBT",
    "payload": {}
}
```

#### HANDLE_EVENT
Acknowledge/clear a pending event.

```json
{
    "type": "HANDLE_EVENT",
    "payload": { "eventId": 42 }
}
```

#### REDUCE_MANPOWER (Event-driven)
Reduce a company's available workforce.

```json
{
    "type": "REDUCE_MANPOWER",
    "payload": { "amount": 10 }
}
```

#### REMOVE_ITEMS (Event-driven)
Remove items from a company's storage (e.g., fire disaster).

```json
{
    "type": "REMOVE_ITEMS",
    "payload": {
        "items": [
            { "id": "core_wood", "amount": 50.0 },
            { "id": "ALL", "amount": 10.0 }
        ]
    }
}
```

#### ADD_DEMAND (Event-driven)
Create artificial buy orders to spike prices (simulates demand shock).

```json
{
    "type": "ADD_DEMAND",
    "payload": {
        "items": [
            { "id": "core_grain", "amount": 10000.0 }
        ]
    }
}
```

#### SET_TICK_RATE / SCHEDULE_EVENT
Placeholder actions (currently no-ops).

---

## 5. Serialized State Format

`getSerializedState()` returns a JSON object:

```json
{
    "turn": 42,
    "date": "2024-01-15",
    "player_id": "550e8400-e29b-41d4-a716-446655440000",
    "entities": [
        {
            "id": "550e8400-e29b-41d4-a716-446655440000",
            "name": "My Company",
            "type": "company",
            "components": {
                "WalletComponent": {
                    "balance": 5000.0,
                    "debt": 0.0
                },
                "InventoryComponent": {
                    "items": {
                        "core_wood": 100.0,
                        "core_stone": 50.0
                    }
                },
                "Storage": {
                    "maxWeight": 5000.0
                }
            }
        }
    ]
}
```

Entity types you will encounter:
- `"company"` — player and AI companies
- `"market"` — market entities with MarketComponent (order books for each item)
- `"factory"` — factory buildings owned by companies
- `"trade_node"` — trade nodes that consume goods
- `"scenario"` — the scenario definition entity

Each entity has an `id` (UUID), `name`, `type`, and `components` map.

### Key Component Fields

| Component | Key Field | Description |
|---|---|---|
| `WalletComponent` | `balance`, `debt` | Company finances |
| `InventoryComponent` / `Storage` | `items` (map of itemId → amount) | Stored goods |
| `MarketComponent` | `orderBooks` (map of itemId → OrderBook) | Buy/sell orders per item |
| `ProductionComponent` | `pipelines`, `inputs`, `outputs` | Factory production lines |
| `AssetOwnerComponent` | `assets` (array of UUIDs) | Owned factories |
| `AIControllerComponent` | `type` | AI brain type |
| `ManpowerPoolComponent` | `totalWorkers`, `availableWorkers` | Workforce |
| `OwnerComponent` | `ownerId` | Who owns this entity |

---

## 6. Event System

`getPendingEvents()` returns an array of pending events:

```json
[
    {
        "id": 1,
        "type": "event_type_name",
        "title": "Market Crash",
        "description": "A financial crisis hits!",
        "choices": [
            {
                "id": "bailout",
                "text": "Bail out companies",
                "effects": { "ADD_DEMAND": { "items": [...] } }
            },
            {
                "id": "ignore",
                "text": "Let the market self-correct",
                "effects": {}
            }
        ]
    }
]
```

After the player makes a choice, send:
```json
{
    "type": "HANDLE_EVENT",
    "payload": { "eventId": 1 }
}
```

And execute the choice's effects by calling `sendInput` with the effects payload (e.g., `ADD_DEMAND`).

---

## 7. Console / Logs

`readConsole()` returns log messages since the last call. Poll this every tick to show engine status in your frontend.

Each string is formatted as:
```
[2024-01-15 12:00:00] [INFO] Engine initialized.
```

For structured logs, use `Console::getLogs()` (returns `std::vector<Log>`) by accessing the engine's `getController().getGamestate()`:

```cpp
auto &ctrl = project_augustus::getController();
auto logs = Console::getLogs(); // struct Log { message, logType, timestamp }
```

---

## 8. Typical Game Loop Flow

```
1. initEngine()
2. loadGameFiles(contents, names)   // load all JSON definitions
3. startScenario("debug_scenario")
4. setPlayer("Player Corp", "template_id", false)
5. loop:
      step()                         // advance 1 tick
      state = getSerializedState()   // get full state
      events = getPendingEvents()    // check for events
      logs = readConsole()           // read log messages
      // if player action:
      sendInput(json_string)         // process input
6. saveGame("quicksave")            // optional
```

---

## 9. Frontend Integration Patterns

### Pattern A: Webview (Built-in)
The existing `main.cpp` uses `webview` library with bindings:
- JS calls `api_step()`, `api_sendInput()`, `api_getSerializedState()`, etc.
- The React/TypeScript frontend at `src/frontend/dist/` communicates via `window.api_*` functions.

### Pattern B: External Process (Vulkan, Custom)
Use the static library directly (see §1). Call `step()` from your render loop and use `getSerializedState()` to update your UI.

### Pattern C: Network-Remote Frontend
You can wrap the engine in a local WebSocket/HTTP server that exposes the API endpoints. Each API function above maps directly to a network call. For example, a REST-like interface:

```
POST /api/step          → step()
POST /api/stepN         → stepN(n)
POST /api/sendInput     → sendInput(json)
GET  /api/state         → getSerializedState()
GET  /api/player        → getPlayerState()
GET  /api/market/:id    → getMarketData(id)
GET  /api/events        → getPendingEvents()
POST /api/save/:name    → saveGame(name)
POST /api/load/:name    → loadGame(name)
GET  /api/saves         → listSaves()
```

---

## 10. Important Notes

- **Thread safety**: The engine is **not** thread-safe. Call all functions from the same thread.
- **Step granularity**: Each `step()` runs one game tick. In a real-time frontend, you might call `step()` at a fixed interval (e.g., every 100ms) or let the user advance manually.
- **Market IDs**: Use the UUID string from the entity's `id` field for `marketId` parameters.
- **Factory IDs**: Use the UUID string from the factory entity's `id` field.
- **Data files**: The mod files (`mock/core/data/`) serve as the reference data set. Your frontend must load these files (or equivalent) via `loadGameFiles` before starting a scenario.

---

*Generated for Project Augustus — see `src/Api/NativeApi.h` and `src/Game/InputHandler.cpp` for the full source.*
