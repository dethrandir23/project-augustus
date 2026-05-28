# Codebase Index

## High-Level Summary
**Project Augustus** is a sophisticated simulation engine built with **C++2int0**. It utilizes a custom **Entity Component System (ECS)** to manage a complex world of economic actors and spatial structures. The project is designed for performance and extensibility, using **CMake** as its build system and supporting both native (OpenGL/GLFW) and web (WebAssembly/Emscripten) targets.

### Core Technologies
- **Language**: C++20
- **Build System**: CMake
- **Architecture**: Entity Component System (ECS)
- **UI**: Dear ImGui (Native) / Webview/React (Planned)
- **Data Serialization**: JSON $\to$ MessagePack $\to$ LZ4 (Compression)

## Architectural Notes

### Entity Component System (ECS)
The heart of the engine is a custom ECS implementation:
- **`Entity`**: A unique object identified by a `uuid`. It acts as a container for various `Component` objects.
- **`Component`**: Data-only classes that define specific attributes (e.g., `InventoryComponent`, `MarketComponent`). All components support JSON serialization.
- **Systems**: Logic-driven classes (e.g., `MarketSystem`) that operate on entities possessing specific components.

### Data Flow & Persistence
The engine employs a multi-stage pipeline for efficient data handling:
1. **Definition**: High-level game data and scenarios are defined in **JSON**.
2. **Serialization**: At runtime, the `Gamestate` is converted to **MessagePack** for a compact binary representation.
3. **Compression**: The binary data is compressed using **LZ4** via `FileUtils` to minimize disk footprint.
4. **Storage**: Compressed save files are stored in a configurable directory.

### User Interface
The UI layer is currently built using **Dear ImGui** for native development and debugging, providing a powerful toolset for inspecting the ECS state and managing the game world in real-time.

## Directory Map

### `src/` - Core Logic
- **`Core/`**: The foundation of the engine.
  - `ECS/`: Implementation of `Entity` and `Component` base classes.
  - `Types.h`: Global type definitions.
- **`Game/`**: High-level gameplay logic and state management.
  - `Gamestate.h`: The primary container for the entire simulation state.
  - `EntityManager.cpp`: Logic for managing the lifecycle of entities.
  - `InputHandler.cpp`: Processes user inputs.
- **`World/`**: Spatial and environmental simulation.
  - `Map.h`: The geographic representation of the game world.
  - `Systems/`: Systems that iterate over entities (e.g., `MarketSystem`).
  - `Components/`: Specific spatial components (e.g., `LocationComponent`).
- **`Economy/`**: Economic simulation systems (Markets, Orderbooks).
- **`AI/`**: AI decision-making and registry.
- **`Registry/`**: Managers for various game-wide data registries (e.g., `ItemManager`, `TechnologyManager`).
- **`IO/`**: Input/Output operations.
  - `SaveUtils.cpp`: Handles serialization, compression, and loading of game saves.
  - `FileUtils.cpp`: Low-level file system operations.
- **`DevTools/`**: Debugging and development utilities (Console, Config loaders).
- **`Api/`**: Interface between the core engine and external layers (Native/Web).
- **`Math/`**: Mathematical utilities (ReLU, Sigint, etc.).

### `data/` - Game Data
- `core/`: Static JSON definitions for mods and initial game state.
- `saves/`: Compressed `.save` files generated during gameplay.

### `config/` - Configuration
- `.toml` files for engine and AI configuration (e.g., `AIConfig.toml`).

### `lib/` - Third-Party Dependencies
- External libraries like `imgui`, `lz4`, `nlohmann/json`, and `uuid`.

## Developer Tips

### Adding a New Component
1. Inherit from `Component` in a new header file in `src/Core/Components/` or `src/World/Components/`.
2. Implement `GetComponentType()`, `ToJson()`, and `UpdateFromJson()`.
3. Register the component logic in the relevant System.

### Debugging
- Use the `Console` utility in `src/DevTools/` to log messages with different severity levels.
- Inspect the ECS state via the ImGui debug windows in the native build.
- Check `data/saves/` to verify if serialization and compression are working as expected.

### Data Flow Reference
When adding new persistent features, ensure the flow follows:
**`JSON` (Definition) $\to$ `Component::ToJson()` $\to$ `MessagePack` $\to$ `LZ4` $\to$ `.save` file.**
