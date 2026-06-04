# PROJECT AUGUSTUS – Development Roadmap

This roadmap defines the staged evolution of PROJECT AUGUSTUS from core simulation engine to fully moddable economic simulation platform.

---

# Phase 1 – Core Stabilization (Engine Foundation)

**Goal:** Complete ECS migration and stabilize deterministic simulation core.

### ECS Completion

* [ ] Implement `EntityManager`
* [ ] Remove all `legacy/` classes
* [ ] Ensure all gameplay logic runs fully on ECS
* [ ] Convert player-only logic to ID-driven multi-company system

### Systems Stabilization

* [ ] Finalize `MarketSystem`
* [ ] Validate orderbook resolution logic
* [ ] Validate population growth & contraction logic
* [ ] Validate production chains

### Code Quality

* [ ] Remove unused includes
* [ ] Run clang-format across project
* [ ] Add Doxygen documentation
* [ ] Clean AI-generated comments

### Ekonomik Dengeleme

* [x] `GameConstants.h` oluşturuldu (`src/Core/GameConstants.h`)
* [ ] Tüm sayısal sabitler tek yerde toplandı (CompanyBrain, TradeNodeBrain, EconomyEvaluator, GameManager, EconomyUtils)
* [x] AI noise sistemi: şirketler %100 optimal değil, `noiseLevel` (0.0-1.0) ile sapma
* [x] `investThreshold`/`investDivisor` artık konfigüre edilebilir (hardcoded 5000/1000 değil)
* [x] `scoreFactoryProfitability` ROI bazlı: `(netProfit / buildCost) * 100`
* [x] `hireWorkers()`: işçi alma maliyeti + budget kontrolü
* [x] Zorluk seviyesi hazırlığı: `noiseLevel` farklılaştırılabilir

### Developer Experience

* [ ] Zorluk seviyesi → noiseLevel + AI bonus mappingi
* [ ] Difficulty enum: Easy / Normal / Hard / VeryHard

### Result of Phase 1

Engine can:

* Load world
* Simulate economy
* Run turns deterministically
* Handle multiple companies correctly

---

# Phase 1.5 – Economic Depth & AI Personality

**Goal:** Make the simulation feel alive and imperfect.

### Prosperity (Refah → Prosperity) Sistemi

* [ ] `DemographicsComponent`'e `prosperity` field'ı ekle (0.0-1.0)
* [ ] Data-driven: `luxury` kategorisindeki itemler prosperity'yi etkiler
* [ ] Hayati değil: prosperity düşükse nüfus azalmaz, sadece refah düşer
* [ ] UI'da prosperity göstergesi, happiness'tan ayrı bir metrik

### AI Kişilik & Noise Sistemi

* [ ] Her şirketin farklı `noiseLevel`, `aggression`, `riskTolerance` değerleri
* [ ] Zorluk seviyesi bu değerleri etkiler (Easy=kaotik, Hard=optimal)
* [ ] Advisor sistemi (oyuncuya özel AI yardımı)
* [ ] Şirketlerin zamanla değişen stratejileri

### Enflasyon Sistemi

* [ ] Data-driven zamanla değişen fiyat çarpanı
* [ ] Tüm fiyatlara uygulanan global enflasyon faktörü
* [ ] JSON'dan yüklenebilir senaryo bazlı enflasyon

### Şehir Ekonomisi

* [ ] İşgücü maliyeti şehre göre değişsin (fabrikayı ucuz işgücü olan yere kurma)
* [ ] Şehirlere özel üretim profilleri (farklı yerler farklı şeyler üretir)

---

# Phase 2 – Serialization & Save System

**Goal:** Make the simulation persistable and version-safe.

### JSON & Component Serialization

* [ ] Implement `to_json` for all Components
* [ ] Implement `from_json`
* [ ] Implement `updateFromJson`

Library:

* nlohmann/json

### Save Manager

* [ ] Implement `SaveManager`
* [ ] Full GameState serialization
* [ ] Partial state sync support

### Save Format

Pipeline:

```
GameState → JSON → MessagePack → LZ4 → Save File
```

Compression:

* LZ4

### Versioning

* [ ] Save file version header
* [ ] Backward compatibility layer

### Result of Phase 2

* Reliable save/load
* Compact save files
* Engine restart-safe

---

# Phase 3 – Script Engine Integration

**Goal:** Make the engine moddable and extensible.

### Script Abstraction

* [ ] Design `IScriptEngine`
* [ ] Implement Lua engine wrapper

Primary target:

* LuaJIT

### Hook System

* [ ] Add `// @hookable` annotations
* [ ] Hook into:

  * Game loop
  * Event dispatch
  * Market execution
  * AI decision points

### API Layer

* [ ] Expose C++ API to Lua
* [ ] Sandbox execution
* [ ] Error isolation

### Type Definitions

* [ ] Auto-generate `producer.d.ts`
* [ ] Provide TypeScript definitions for scripts

### Result of Phase 3

* Gameplay logic partially scriptable
* Mods can inject logic
* Event behavior customizable

---

# Phase 4 – Advanced Gameplay Systems

**Goal:** Deepen simulation realism.

### Legal System Expansion

* [ ] Court system
* [ ] Multi-step legal resolution
* [ ] Reputation impact system

### Government & War

* [ ] State entities in ECS
* [ ] War economy modifiers
* [ ] Trade embargo system

### Finance System

* [ ] IPO mechanics
* [ ] Stock market orderbooks
* [ ] News-driven volatility

### AI Improvements

* [ ] Company AI strategies
* [ ] Risk evaluation
* [ ] Bankruptcy handling

Optional:
ML inference experiments via:

* Python

### Result of Phase 4

Game becomes fully playable sandbox simulation.

---

# Phase 5 – UI Evolution

**Goal:** Transition from debug UI to production-grade interface.

### Native Debug UI

Using:

* Dear ImGui

* [ ] Complete panel system

* [ ] Event notification system

* [ ] Debug overlays

### Web UI (Hybrid Architecture)

Frontend stack:

* React
* Tailwind CSS

Rendering options:

* webview

* WebSocket bridge

* [ ] Real-time data sync

* [ ] Market dashboards

* [ ] Company management panels

* [ ] Advanced analytics views

### Result of Phase 5

* Professional-grade interface
* Analytics-heavy UI
* External rendering capability

---

# Phase 6 – Headless Mode & Dedicated Simulation

**Goal:** Separate simulation from rendering.

* [ ] Headless CLI mode
* [ ] Simulation-only binary
* [ ] Deterministic replay system
* [ ] External control via WebSocket API
* [ ] AI training integration mode

Build system:

* CMake

### Result of Phase 6

* Server-ready engine
* Multiplayer foundation
* AI experimentation platform

---

# Phase 7 – Optimization & Scaling

**Goal:** Make the engine large-world capable.

### Performance

* [ ] Memory profiling
* [ ] Cache-friendly ECS storage
* [ ] Parallel system updates
* [ ] Large-scale market simulation test

### Stress Testing

* [ ] 1000+ TradeNodes
* [ ] 500+ Companies
* [ ] High-frequency event bursts

### Result of Phase 7

Engine scales to grand-strategy-level simulations.

---

# Phase 8 – Modding Ecosystem

**Goal:** Turn PROJECT AUGUSTUS into a moddable platform.

* [ ] Mod sandboxing
* [ ] Dependency resolution
* [ ] Mod version compatibility
* [ ] Public API documentation site
* [ ] Sample mods repository

Long-term:

* Steam Workshop style system
* Scenario marketplace

---

# Long-Term Vision

PROJECT AUGUSTUS aims to become:

* A deterministic economic simulation engine
* A moddable sandbox
* A research-friendly economy simulation tool
* A multiplayer-capable strategy platform

The engine core must always remain:

* Simulation-first
* UI-agnostic
* Script-agnostic
* Deterministic
* Extensible

---