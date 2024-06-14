cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(ldt-rosatom VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# set(CMAKE_BUILD_TYPE Release)

add_subdirectory(third-party/OpenXLSX)

add_executable(scheduler
    main.cpp
    src/parser.cpp
    src/algos/greedy.cpp
    src/path.cpp
)

target_include_directories(scheduler PUBLIC src)
target_include_directories(scheduler PUBLIC third-party/boost)
target_include_directories(scheduler PUBLIC third-party)
target_include_directories(scheduler PUBLIC third-party/boost/graph/include)
target_link_libraries(scheduler OpenXLSX::OpenXLSX)
