cmake_minimum_required(VERSION 3.15)

project(project-augustus LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/App
    #${CMAKE_SOURCE_DIR}/src/App/UI
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/imgui
)

file(COPY ${CMAKE_SOURCE_DIR}/fonts DESTINATION ${CMAKE_BINARY_DIR})

file(GLOB_RECURSE APP_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE IMGUI_SOURCES "${CMAKE_SOURCE_DIR}/lib/imgui/*.cpp")

# these three lines added for prevent legacy imgui code from compiling, so remove them later.
file(GLOB_RECURSE UI_SOURCES "${CMAKE_SOURCE_DIR}/src/App/UI/*.cpp")
list(REMOVE_ITEM APP_SOURCES ${UI_SOURCES})

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

if(NOT EMSCRIPTEN)
    target_link_libraries(project-augustus PRIVATE ${NATIVE_LIBS})
endif()