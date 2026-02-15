# Producer TODO List

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
- [ ] Write a UI manager class.
- [ ] Add an input window to the UI.
- [ ] Add an window organiser to the UI.

## Systems & API
- [ ] Design and implement a localization and string/text filling system.
- [ ] Add a configuration file and system for project settings (e.g., file logging, window name, debug mode, font settings).
- [ ] Integrate toml++ into project and provide a config API.
- [ ] Learn about the OneLight API and add relevant files (e.g., `OneLightApi.h`).
