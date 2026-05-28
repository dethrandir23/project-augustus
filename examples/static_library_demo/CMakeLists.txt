cmake_minimum_required(VERSION 3.15)
project(engine_demo)

set(CMAKE_CXX_STANDARD 20)

add_executable(engine_demo main.cpp)
target_link_libraries(engine_demo PRIVATE
    /home/efe/Documents/projects/project-augustus/build/lib/libaugustus_engine.a
)
target_include_directories(engine_demo PRIVATE
    /home/efe/Documents/projects/project-augustus/src
)
