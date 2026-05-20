cmake_minimum_required(VERSION 3.15)

project(project-augustus LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/App
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/imgui
)

file(COPY ${CMAKE_SOURCE_DIR}/fonts DESTINATION ${CMAKE_BINARY_DIR})

file(GLOB_RECURSE APP_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE IMGUI_SOURCES "${CMAKE_SOURCE_DIR}/lib/imgui/*.cpp")

# prevent legacy imgui code
file(GLOB_RECURSE UI_SOURCES "${CMAKE_SOURCE_DIR}/src/App/UI/*.cpp")
list(REMOVE_ITEM APP_SOURCES ${UI_SOURCES})

include(FetchContent)

# ── webview ──
FetchContent_Declare(
    webview
    GIT_REPOSITORY https://github.com/webview/webview
    GIT_TAG 0.12.0
)
FetchContent_MakeAvailable(webview)

if(EMSCRIPTEN)
    set_target_properties(project-augustus PROPERTIES SUFFIX ".js")
    target_link_options(project-augustus PRIVATE
        "--bind" "-sWASM=1" "-sALLOW_MEMORY_GROWTH=1" "-sMODULARIZE=1"
        "-sEXPORT_NAME='GameModule'" "-sNO_EXIT_RUNTIME=1" "-sASSERTIONS=1"
        "-sUSE_GLFW=3"
    )
else()
    message(STATUS "Build Type: Native (Linux/Win/Mac)")
    list(FILTER APP_SOURCES EXCLUDE REGEX "WebApi.cpp$")

    find_package(OpenGL REQUIRED)
    find_package(glfw3 QUIET)

    set(NATIVE_LIBS glfw GL uuid lz4)
endif()

add_executable(project-augustus ${APP_SOURCES} ${IMGUI_SOURCES})

target_link_libraries(project-augustus PRIVATE webview::core)

if(NOT EMSCRIPTEN)
    target_link_libraries(project-augustus PRIVATE ${NATIVE_LIBS})
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
)

include(Catch)
catch_discover_tests(augustus_tests)