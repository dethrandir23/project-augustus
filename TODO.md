# Producer TODO List

## Entity Component System (ECS) & Refactoring
- [ ] Design and implement a virtual `Component` base class.
- [ ] Create a `ComponentSystem` to manage components.
- [ ] Develop core components (e.g., `TransformComponent`, `DataComponent`) based on the new `Component` class.
- [ ] Design and implement a virtual `Entity` base class.
- [ ] Create an `EntityManager` to manage entities.
- [ ] Refactor `Market`, `TradeNode`, `Company`, and `Factory` classes into `Entity`-based classes.
- [ ] Decompose `Market`, `TradeNode`, `Company`, and `Factory` logic into specific components.

## Scripting Engine & API
- [ ] Design and implement a generic `IScriptEngine` interface for scripting language integration.
- [ ] Implement a concrete scripting engine (e.g., using Lua, or a JavaScript engine like V8 or QuickJS).
- [ ] Identify and expose hookable functions in the game's control paths (e.g., game loop, event handling, UI interactions).
- [ ] Add special comments (e.g., `// @hookable`) to mark functions exposed to the scripting API for easy identification and documentation.
- [ ] Create a `producer.d.ts` file to provide TypeScript definitions and intellisense for the scripting API.
- [ ] Develop a tool to automatically generate the `producer.d.ts` file by parsing the C++ source for hookable function comments.

## Code Quality & Refactoring
- [ ] Edit and organize includes, removing unnecessary ones.
- [ ] Use clang-formatter to format all files in a single style.
- [ ] Remove AI-generated comment lines.
- [ ] Organize comments and add Doxygen instructions to functions, files, and other elements.
- [ ] Organize file structure.
- [ ] Inspect all diffs and commit them.

## Core Gameplay & Features
- [ ] Convert all player-only input functions to an ID-driven system to support all companies, not just the player's.
- [ ] Test orderbook and other inputs.
- [ ] Research media playing in C++:
    - [ ] Investigate media loading and playing libraries.
    - [ ] Understand how sound effect and music systems work in games.

## UI Development
- [x] Write a UI manager class.
- [ ] Add an input window to the UI.
- [x] Add an window organiser to the UI.

## Systems & API
- [ ] Design and implement a localization and string/text filling system.
- [ ] Add a configuration file and system for project settings (e.g., file logging, window name, debug mode, font settings).
- [x] Integrate toml++ into project and provide a config API.
- [ ] Learn about the OneLight API and add relevant files (e.g., `OneLightApi.h`).