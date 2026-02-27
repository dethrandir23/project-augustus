# PROJECT AUGUSTUS – Architecture

## 1. Architectural Overview

PROJECT AUGUSTUS is a **data-driven, ECS-based economic simulation engine** written primarily in C++.

The architecture follows these principles:

* Entity Component System (ECS) core
* Data-driven game design (JSON-defined world)
* Scriptable runtime (LuaJIT integration planned)
* Hybrid UI architecture (Native ImGui + Web-based React UI)
* Deterministic simulation loop
* Mod-friendly structure
* Serialization-ready design

---

## 2. High-Level Architecture

```
                ┌────────────────────┐
                │   React Frontend   │
                │ (HTML/CSS/TS)      │
                └──────────┬─────────┘
                           │
                   WebSocket / webview.h
                           │
                ┌──────────▼─────────┐
                │      C++ Core      │
                │  (Simulation ECS)  │
                ├──────────┬─────────┤
                │ Script Engine      │
                │   (LuaJIT)         │
                ├──────────┬─────────┤
                │   Data Layer       │
                │ JSON / MsgPack     │
                └──────────┬─────────┘
                           │
                       Save Files
                       (LZ4)
```

---

## 3. Core Technology Stack

### Backend / Core Engine

* C++
* Build System: CMake
* JSON Library: nlohmann/json
* Immediate UI: Dear ImGui
* Audio: miniaudio
* CLI parsing: CLI11

---

### Frontend (Planned Hybrid UI)

* HTML
* CSS
* TypeScript
* React
* Tailwind CSS

Rendering strategies:

* webview (native embedding)
* WebSockets bridge (real-time sync)

---

### Scripting Layer (Planned)

* LuaJIT
* Hookable C++ API
* TypeScript definition generation (`producer.d.ts`)
* Script-level mod support

Goal:

* Gameplay extensibility
* Event injection
* AI behavior overrides
* Custom scenarios

---

### AI & Machine Learning

* Python
* MLP experiments (ReLU, Sigmoid, Softmax already present in Math/)
* Training outside engine
* Runtime inference possible via integration layer

This layer is intentionally separated from deterministic simulation.

---

### Data & Persistence

Game Data:

* JSON (static definitions)
* Located in `data/core/`

Runtime Save:

* MessagePack
* Compressed via LZ4

Design Goals:

* Fast save/load
* Partial state updates
* Version migration support

---

## 4. Core Engine Architecture

### 4.1 Entity Component System (ECS)

The engine uses a custom ECS architecture.

Core structure:

```
Entity
 ├── Components
 │     ├── WalletComponent
 │     ├── ProductionComponent
 │     ├── MarketComponent
 │     ├── WorkforceComponent
 │     └── ...
 └── Systems operate on Components
```

Located in:

* `Core/ECS`
* `Economy/Components`
* `World/Components`
* `AI/Components`

Design goals:

* High decoupling
* Data-oriented design
* Easy serialization
* Script-level exposure

---

### 4.2 Game Simulation Flow

Main loop:

```
Input → Systems Update → Economy Resolution → Events → AI → UI Sync
```

Important subsystems:

* GameManager
* EventHandler
* MarketSystem
* Orderbook resolution
* Population update
* Legal resolution

Simulation is turn-based and deterministic.

---

### 4.3 Economy Engine

The economy is based on:

* TradeNodes
* Markets
* Orderbooks
* Production chains
* Logistics routes

Orderbook logic:

* Separate per good
* Last transaction defines price
* Supply-demand driven

Economic state lives inside ECS components.

---

### 4.4 Mod System

Structure:

```
data/core/
mock/
mod.json
load_order.json
```

Features:

* JSON-driven definitions
* Load order system
* Namespace system
* Planned script injection support

Long-term goal:
Fully moddable economy simulation.

---

## 5. UI Architecture

Two-layer approach:

### 1. Native Layer

* Dear ImGui-based debug UI
* DevTools
* Console
* In-engine panels

### 2. Web UI (Planned)

* React dashboard
* Advanced visualization
* Data tables
* Charts
* Real-time sync via WebSockets

Reason:
Native for dev speed, Web for production-grade UI.

---

## 6. Serialization Design

Each:

* Component
* Entity
* GameState

Will implement:

* `to_json`
* `from_json`
* `updateFromJson`

Save Pipeline:

```
GameState → JSON → MessagePack → LZ4 → Save File
```

Goals:

* Fast
* Compact
* Backward compatible

---

## 7. Scripting Architecture (Planned)

Interface:

```
IScriptEngine
  ├── LuaScriptEngine
```

Features:

* Hookable engine functions (`// @hookable`)
* Game loop hooks
* Event interception
* Custom AI injection
* API documentation auto-generation

Future possibility:
Sandboxed script execution.

---

## 8. Networking (Planned)

Possible WebSocket server inside engine:

Use cases:

* Web UI communication
* Remote monitoring
* Multiplayer experiments
* AI external control

---

## 9. Directory Philosophy

```
src/
 ├── Core        → ECS & base types
 ├── Game        → High-level gameplay logic
 ├── Economy     → Economy systems
 ├── World       → Map & TradeNodes
 ├── Registry    → Data managers
 ├── App         → Application layer
 ├── DevTools    → Debug & console
 └── AI          → AI components
```

Data separation:

```
data/      → Real content
mock/      → Test scenarios
metadata/  → Engine metadata
```

---

## 10. Long-Term Architectural Goals

* Full ECS migration (legacy removal)
* Script-driven gameplay
* Headless simulation mode
* Dedicated server mode
* AI training integration
* Deterministic replay system
* Multiplayer-ready core

---

## 11. Design Philosophy

PROJECT AUGUSTUS is designed as:

* A simulation-first engine
* Not UI-driven
* Deterministic
* Extensible
* Moddable
* Scriptable
* Scalable

The core engine must remain independent from:

* UI framework
* Scripting language
* AI implementation
* Rendering strategy

Everything is a plugin layer around the simulation core.

---