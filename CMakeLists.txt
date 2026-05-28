cmake_minimum_required(VERSION 3.15)

project(project-augustus LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# # fastnoise
# set(FASTNOISE2_NOISETOOL OFF CACHE BOOL "Build Noise Tool" FORCE)
# add_subdirectory(deps/FastNoise2)

option(BUILD_AS_STATIC_LIB "Build only the static library" OFF)
option(BUILD_GODOT_PLUGIN "Build Godot GDExtension plugin (.so)" OFF)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/App
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/imgui
)

file(COPY ${CMAKE_SOURCE_DIR}/fonts DESTINATION ${CMAKE_BINARY_DIR})

file(GLOB_RECURSE ALL_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE IMGUI_SOURCES "${CMAKE_SOURCE_DIR}/lib/imgui/*.cpp")

file(GLOB_RECURSE UI_SOURCES "${CMAKE_SOURCE_DIR}/src/App/UI/*.cpp")
list(REMOVE_ITEM ALL_SOURCES ${UI_SOURCES})

list(FILTER ALL_SOURCES EXCLUDE REGEX "main\\.cpp$")

# Godot sources are only for the GDExtension plugin build
list(FILTER ALL_SOURCES EXCLUDE REGEX "src/Godot/")
set(ENGINE_SOURCES ${ALL_SOURCES})

include(FetchContent)

# ── webview (only linked by the native executable) ──
FetchContent_Declare(
    webview
    GIT_REPOSITORY https://github.com/webview/webview
    GIT_TAG 0.12.0
)
FetchContent_MakeAvailable(webview)

if(EMSCRIPTEN)
    message(STATUS "Build Type: Emscripten/WASM")
    set(EMSCRIPTEN_FLAGS
        "--bind" "-sWASM=1" "-sALLOW_MEMORY_GROWTH=1" "-sMODULARIZE=1"
        "-sEXPORT_NAME='GameModule'" "-sNO_EXIT_RUNTIME=1" "-sASSERTIONS=1"
        "-sUSE_GLFW=3"
    )
else()
    message(STATUS "Build Type: Native (Linux/Win/Mac)")
    list(FILTER ENGINE_SOURCES EXCLUDE REGEX "WebApi.cpp$|NativeApi\\.cpp$")

    # glfw/GL only needed by the webview-based executable, not by the engine lib itself
    set(CORE_LIBS uuid lz4)

    if(NOT BUILD_GODOT_PLUGIN)
        find_package(OpenGL REQUIRED)
        find_package(glfw3 QUIET)
        set(APP_LIBS glfw GL)
    endif()
endif()

# ── Engine Static Library ──
add_library(augustus_engine STATIC ${ENGINE_SOURCES} ${IMGUI_SOURCES})

if(NOT EMSCRIPTEN)
    target_link_libraries(augustus_engine PUBLIC ${CORE_LIBS})
endif()

# ── Godot GDExtension Plugin ──
if(BUILD_GODOT_PLUGIN)
    message(STATUS "Building Godot GDExtension plugin")

    FetchContent_Declare(
        godot-cpp
        GIT_REPOSITORY https://github.com/godotengine/godot-cpp
        GIT_TAG godot-4.3-stable
    )
    FetchContent_MakeAvailable(godot-cpp)

    file(GLOB_RECURSE GODOT_SOURCES "${CMAKE_SOURCE_DIR}/src/Godot/*.cpp")

    set(GODOT_ENGINE_SOURCES ${ENGINE_SOURCES})
    list(REMOVE_ITEM GODOT_ENGINE_SOURCES ${IMGUI_SOURCES})

    # add_library(project-augustus-godot SHARED ${GODOT_SOURCES} ${GODOT_ENGINE_SOURCES} ${source}) # fastnoise added
    # target_link_libraries(project-augustus-godot PRIVATE godot-cpp ${CORE_LIBS} PUBLIC Fastnoise) # fastnoise added
    add_library(project-augustus-godot SHARED ${GODOT_SOURCES} ${GODOT_ENGINE_SOURCES})
    target_link_libraries(project-augustus-godot PRIVATE godot-cpp ${CORE_LIBS})

    set_target_properties(project-augustus-godot PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/addons/project-augustus"
    )
endif()

# ── Native Executable ──
if(EMSCRIPTEN)
    add_executable(project-augustus "${CMAKE_SOURCE_DIR}/src/App/main.cpp")
    target_link_libraries(project-augustus PRIVATE augustus_engine)
    set_target_properties(project-augustus PROPERTIES SUFFIX ".js")
    target_link_options(project-augustus PRIVATE ${EMSCRIPTEN_FLAGS})
elseif(NOT BUILD_AS_STATIC_LIB AND NOT BUILD_GODOT_PLUGIN)
    add_executable(project-augustus "${CMAKE_SOURCE_DIR}/src/App/main.cpp")
    target_link_libraries(project-augustus PRIVATE augustus_engine webview::core ${APP_LIBS})
elseif(BUILD_AS_STATIC_LIB)
    message(STATUS "BUILD_AS_STATIC_LIB=ON: Skipping executable, building only the static library.")
endif()

# ── TESTS ──
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.3
)
FetchContent_MakeAvailable(Catch2)

enable_testing()

add_executable(augustus_tests
  tests/test_economy.cpp
  tests/test_events.cpp
  tests/test_market.cpp
  tests/test_saveload.cpp
)

target_link_libraries(augustus_tests
  PRIVATE
    Catch2::Catch2WithMain
    augustus_engine
)

include(Catch)
catch_discover_tests(augustus_tests)

# ── Headless Test ──
if(NOT EMSCRIPTEN AND NOT BUILD_GODOT_PLUGIN)
    add_executable(headless_test tests/headless_test.cpp)
    target_link_libraries(headless_test PRIVATE augustus_engine)
    target_compile_definitions(headless_test PRIVATE DATA_ROOT="${CMAKE_SOURCE_DIR}/data/core/data")

    if(NOT BUILD_AS_STATIC_LIB)
        add_test(NAME headless_test
            COMMAND headless_test 100 3 "${CMAKE_SOURCE_DIR}/data/core/data"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endif()
endif()
