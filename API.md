# Project Augustus — Game Engine API Reference

> For frontend developers integrating with the C++ game engine.
> Last updated: 2026-05

---

## Architecture

```
Frontend (React/Vue/ImGui/WebView)
        │
        ├── Push notifications (C++ → JS): engine_ready, tick_complete, stepN_complete
        │      Registered via setJsCallback(type, data)
        │
        └── Pull queries (JS → C++): getSerializedState(), getPlayerState(), etc.
               Called directly on WASM exports or via webview bindings
```

The engine runs independently from the UI. The frontend communicates through **two channels**:

| Direction | Mechanism | Purpose |
|-----------|-----------|---------|
| C++ → JS | Push events via callback | Lightweight notifications (tick done, engine ready) |
| JS → C++ | Direct function calls | Heavy state queries (serialized game state, market data) |

---

## Namespace: `project_augustus`

All public API functions are declared in the `project_augustus` namespace in `src/Api/NativeApi.h`.

For **WASM builds**, these are exposed as individual JavaScript functions via Emscripten bindings (see [WASM Bindings](#wasm-webapi-bindings)).

For **native webview builds**, they are bound via `webview::webview::bind()` in `main.cpp` with an `api_` prefix.

---

## API Reference

### Initialization & Lifecycle

#### `void initEngine()`
Initializes the engine: registers all JSON data handlers (`ItemManager`, `PipelineManager`, `FactoryManager`, etc.), initializes `InputHandler` and `AIRegistry`.
- **Call once** at startup before any other API function.
- **WASM**: Fires `engine_ready` event after initialization.

---

### Data Loading

#### `bool loadGameFiles(contents, names)`
Loads game data files into the engine registries.
- **Parameters:**
  - `contents` — `vector<string>` — File contents as strings
  - `names` — `vector<string>` — Corresponding file names (for error reporting)
- **Returns:** `true` if all files loaded successfully
- **WASM**: Fires `files_loaded` event.

#### `bool startScenario(scenarioId)`
Loads a scenario by ID from registered scenario definitions. Creates the world (TradeNodes, Markets, Companies).
- **Parameters:**
  - `scenarioId` — `string` — e.g. `"debug_scenario"`
- **Returns:** `true` if the scenario was found and loaded
- **WASM**: Fires `scenario_loaded` event.

#### `void setPlayer(name, templateId, isAI)`
Creates the player's company entity and assigns it to the player.
- **Parameters:**
  - `name` — `string` — Company display name (e.g. `"Efe Ticaret A.S."`)
  - `templateId` — `string` — Template key from `CompanyManager::templates` (e.g. `"core_startup_company_001"`)
  - `isAI` — `bool` — If `true`, adds an `AIControllerComponent` for autonomous play
- **WASM**: Fires `player_created` event with the player state JSON.

---

### Game Loop

#### `void step()`
Runs one simulation tick. This:
1. Processes all pending inputs
2. Runs AI controllers
3. Ticks markets (order matching, price updates)
4. Ticks factories (production, consumption)
5. Ticks trade nodes (supply/demand propagation)
6. Checks event triggers
- **WASM**: Fires `tick_complete` event with the full serialized state.

#### `void stepN(n)`
Runs N consecutive ticks.
- **Parameters:**
  - `n` — `size_t` — Number of ticks to simulate
- Use for fast-forward or AI-only simulations.
- **WASM**: Fires `stepN_complete` event with the full serialized state.

---

### State Queries (Pull)

All query functions return `std::string` containing JSON.

#### `string getSerializedState()`
Returns the **full game state** as a JSON string.
- **Heavy operation** — do not call every frame. Use for save/thick debug views.
- Format:
  ```json
  {
    "turn": 47,
    "date": "17/2/1836",
    "player_id": "uuid-string",
    "entities": [
      { "id": "uuid", "type": "company", "name": "...", "components": { ... } },
      { "id": "uuid", "type": "market", "name": "...", "components": { ... } },
      { "id": "uuid", "type": "factory", "name": "...", "components": { ... } },
      { "id": "uuid", "type": "trade_node", "name": "...", "components": { ... } }
    ]
  }
  ```

#### `string getPlayerState()`
Returns the **player's company entity** as a JSON string.
- Lighter than `getSerializedState()`. Call after every tick to refresh dashboard.
- Format:
  ```json
  {
    "id": "uuid",
    "name": "Efe Ticaret A.S.",
    "type": "company",
    "components": {
      "WalletComponent": { "balance": 12500.0, "debt": 0.0 },
      "MainStorage": { "items": [{ "id": "core_grain_005", "qty": 250 }] },
      "ManpowerPoolComponent": { "availableWorkers": 80, "totalWorkers": 100 }
    }
  }
  ```

#### `string getMarketData(marketId)`
Returns **all market entities** as a JSON array string.
- **Parameters:**
  - `marketId` — `string` — Currently unused (returns all markets). Pass any non-empty string.
- Format:
  ```json
  [
    { "id": "uuid", "type": "market", "name": "Northern Market", "components": { ... } },
    { "id": "uuid", "type": "market", "name": "Southern Market", "components": { ... } }
  ]
  ```

#### `string getFactoryStatus(factoryId)`
Returns a **single factory entity** as a JSON string.
- **Parameters:**
  - `factoryId` — `string` — UUID of the factory entity
- Returns `"{}"` if the factory is not found or the UUID is invalid.
- Format:
  ```json
  {
    "id": "uuid",
    "type": "factory",
    "name": "Grain Farm #1",
    "components": {
      "FactoryComponent": {
        "templateId": "core_factory_grain_001",
        "currentProduction": 0.0,
        "isProducing": true,
        "efficiency": 1.0
      }
    }
  }
  ```

#### `string getPendingEvents()`
Returns the **event queue** as a JSON array string. Events with `auto_handle: false` must be responded to by the player.
- Format:
  ```json
  [
    {
      "id": 3,
      "name": "Fire in Warehouse",
      "description": "A fire has broken out...",
      "event_type": "decision",
      "options": [
        { "index": 0, "text": "Fight the fire" },
        { "index": 1, "text": "Let it burn" }
      ],
      "remained_steps": 3
    }
  ]
  ```

---

### Input / Actions

#### `bool sendInput(inputJson)`
Sends an action to the engine. All player actions use this single entry point.
- **Parameters:**
  - `inputJson` — `string` — JSON string with `type` and `payload` fields
- **Returns:** `true` if the input was valid and processed

**Action Reference:**

| Action Type | Purpose | Payload Fields |
|---|---|---|
| `STEP_GAME` | Advance N turns | `{ times: number }` |
| `ADD_MONEY` | Debug add money | `{ amount: number, companyId?: string }` |
| `ADD_ITEM` | Debug add items | `{ itemId: string, amount: number, companyId?: string }` |
| `BUILD_FACTORY` | Build a factory | `{ templateId: string, customName?: string, companyId?: string }` |
| `SCRAP_FACTORY` | Remove a factory | `{ factoryId: string }` |
| `MARKET_BUY_ITEM` | Place buy order | `{ marketId: string, itemId: string, amount: number, price: number, companyId?: string }` |
| `MARKET_SELL_ITEM` | Place sell order | `{ marketId: string, itemId: string, amount: number, price: number, companyId?: string }` |
| `REDUCE_MANPOWER` | Reduce workforce | `{ amount: number, companyId?: string }` |
| `REMOVE_ITEMS` | Remove inventory | `{ items: [{ id: string, amount: number }], companyId?: string }` |
| `ADD_DEMAND` | Simulation add demand | `{ items: [{ id: string, amount: number }] }` |
| `PAY_DEBT` | Pay off debt | `{ companyId?: string }` |

- `companyId` is optional. If omitted, the player's company is used.
- The AI system always includes its own companyId.

---

### Console

#### `vector<string> readConsole()`
Returns all pending console log messages and **clears the buffer**.
- Call after each tick to drain the log.
- Each call returns only new messages since the last call.

#### `void logToConsole(msg, type)`
Directly writes a message to the engine console.
- **Parameters:**
  - `msg` — `string` — Message text
  - `type` — `LogType` — Enum: `INFO`, `WARNING`, `ERROR`

---

### Save / Load

#### `bool saveGame(name)`
Saves the current game state to disk.
- **Parameters:**
  - `name` — `string` — Save file name (without extension)
- **Returns:** `true` on success
- Format: LZ4-compressed MessagePack

#### `bool loadGame(name)`
Loads a previously saved game state from disk.
- **Parameters:**
  - `name` — `string` — Save file name (without extension)
- **Returns:** `true` on success

#### `vector<string> listSaves()`
Lists all available save names.
- Returns an array of save name strings (without extensions).

---

### Callback Registration

#### `void registerCallback(cb)`
Registers a callback function for engine-to-frontend push notifications.
- **Parameters:**
  - `cb` — `EngineCallback` — Signature: `void(const string& eventType, const nlohmann::json& payload)`
- **Note:** The current implementation is a stub. The WASM build uses a separate `setJsCallback()` mechanism.

---

## WASM / WebAPI Bindings

When compiled with Emscripten (`EMSCRIPTEN=1 cmake ...`), the engine is exposed to JavaScript via the bindings in `src/Api/WebApi.cpp`.

### Module Loading

The WASM module is exposed as `window.GameModule` (a factory function via `-sMODULARIZE=1 -sEXPORT_NAME='GameModule'`).

```js
const script = document.createElement("script");
script.src = "/project-augustus.js";
script.onload = async () => {
  const mod = await window.GameModule();
  // mod now has all exported functions
  mod.initEngine();
};
```

### JavaScript-Exposed Functions

All functions below are directly callable on the module instance:

| Function | Signature | Returns | Fires Event |
|---|---|---|---|
| `setJsCallback` | `(callback: (type: string, data: string) => void) => void` | — | — |
| `initEngine` | `() => void` | — | `engine_ready` |
| `loadGameFiles` | `(contents: string[], names: string[]) => boolean` | `boolean` | `files_loaded` |
| `startScenario` | `(scenarioId: string) => boolean` | `boolean` | `scenario_loaded` |
| `setPlayer` | `(name: string, templateId: string, isAI: boolean) => void` | — | `player_created` |
| `step` | `() => void` | — | `tick_complete` |
| `stepN` | `(n: number) => void` | — | `stepN_complete` |
| `getSerializedState` | `() => string` | JSON string | — |
| `getPlayerState` | `() => string` | JSON string | — |
| `getMarketData` | `(marketId: string) => string` | JSON array string | — |
| `getFactoryStatus` | `(factoryId: string) => string` | JSON string | — |
| `getPendingEvents` | `() => string` | JSON array string | — |
| `sendInput` | `(inputJson: string) => boolean` | `boolean` | — |
| `readConsole` | `() => string[]` | `string[]` | — |
| `logToConsole` | `(msg: string) => void` | — | — |
| `saveGame` | `(name: string) => boolean` | `boolean` | — |
| `loadGame` | `(name: string) => boolean` | `boolean` | — |
| `listSaves` | `() => string[]` | `string[]` | — |

### Push Event Types

Events fired via the callback registered with `setJsCallback`:

| Event Type | Triggered By | Payload |
|---|---|---|
| `engine_ready` | `initEngine()` | `"{}"` |
| `files_loaded` | `loadGameFiles()` | `"true"` / `"false"` |
| `scenario_loaded` | `startScenario()` | `"true"` / `"false"` |
| `player_created` | `setPlayer()` | Player state JSON |
| `tick_complete` | `step()` | Full serialized state JSON |
| `stepN_complete` | `stepN()` | Full serialized state JSON |

### Exposed C++ Types (for advanced use)

The following C++ types are also bound for JavaScript access:

- **`Console`** class with static methods: `help()`, `parseInput()`, `log()`, `readLog()`, `getLog()`, `clearLogs()`, `removeLog()`, `getLogCount()`, `getLogs()`, `getLogMessages()`
- **`LogType`** enum: `INFO`, `ERROR`, `WARNING`
- **`Log`** value object: `message`, `logType`, `timestamp`
- **`StringList`** vector type: `vector<string>`

---

## Communication Pattern

### Recommended Frontend Flow

```
1. Load WASM module (dynamic script tag → GameModule factory)
2. Set JS callback: mod.setJsCallback((type, data) => { ... })
3. Initialize: mod.initEngine()
   └── receives: "engine_ready"
4. Load data: mod.loadGameFiles(contents, names)
   └── receives: "files_loaded"
5. Start: mod.startScenario("debug_scenario")
   └── receives: "scenario_loaded"
6. Set player: mod.setPlayer("Company Name", "template_id", false)
   └── receives: "player_created"
7. Game loop (on user action or timer):
   └── mod.step()
       └── receives: "tick_complete" (with full state)
       └── mod.getPlayerState() → refresh dashboard
       └── mod.getPendingEvents() → show event modals
       └── mod.readConsole() → update console log
```

### State Update Strategy

| Data | When to Fetch | Frequency |
|---|---|---|
| Player state | After each `tick_complete` | Every tick |
| Full state | Save/thick debug | On demand |
| Market data | Market tab open | On tab switch, after tick |
| Factory status | Factory tab open | On tab switch, after tick |
| Events | After each `tick_complete` | Every tick |
| Console | After each `tick_complete` | Every tick |

---

## Build Targets

### Static Library (for embedding)

```bash
cmake -B build -DBUILD_AS_STATIC_LIB=ON
cmake --build build
# Produces: build/lib/libaugustus_engine.a
```

Link with your application and include `<Api/NativeApi.h>`. Call functions via `project_augustus::` namespace.

### Executable (desktop app with webview)

```bash
cmake -B build
cmake --build build
# Produces: build/bin/project-augustus
```

### WASM (for web)

```bash
EMSCRIPTEN=1 cmake -B build_wasm
cmake --build build_wasm
# Produces: build_wasm/bin/project-augustus.js + .wasm
```
