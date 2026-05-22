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
