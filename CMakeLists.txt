cmake_minimum_required(VERSION 3.28)
project(SortingCentral LANGUAGES CXX)

# ── C++23 modules ─────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# Experimental module support (Clang 17+ / GCC 14+ / MSVC 19.36+)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-fmodules -fmodule-file-map-prefix=)
endif()

# ── SFML ──────────────────────────────────────────────────────────────────────
# Install: sudo apt install libsfml-dev  /  brew install sfml  /  vcpkg install sfml
find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

# ── sources ───────────────────────────────────────────────────────────────────
add_executable(sorting_central
    main.cpp

    # feature modules
    features/sort_types.cppm
    features/time_misc.cppm
    features/memory_misc.cppm

    # visualiser module
    visualiser/sfml_visualiser.cppm

    # algorithm modules
    algorithms/bubble_sort.cppm
    algorithms/insertion_sort.cppm
    algorithms/merge_sort.cppm
    algorithms/quick_sort.cppm
    algorithms/heap_sort.cppm
    algorithms/bogo_sort.cppm
    algorithms/lsd_radix_sort.cppm
    algorithms/msd_radix_sort.cppm
    algorithms/shell_sort.cppm
)

# Mark all .cppm files as C++ module sources
set_source_files_properties(
    features/sort_types.cppm
    features/time_misc.cppm
    features/memory_misc.cppm
    visualiser/sfml_visualiser.cppm
    algorithms/bubble_sort.cppm
    algorithms/insertion_sort.cppm
    algorithms/merge_sort.cppm
    algorithms/quick_sort.cppm
    algorithms/heap_sort.cppm
    algorithms/bogo_sort.cppm
    algorithms/lsd_radix_sort.cppm
    algorithms/msd_radix_sort.cppm
    algorithms/shell_sort.cppm
    PROPERTIES
        CXX_SCAN_FOR_MODULES ON
)

target_link_libraries(sorting_central PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)

# ── compiler-specific hardening ───────────────────────────────────────────────
if(MSVC)
    target_compile_options(sorting_central PRIVATE /W4 /permissive-)
else()
    target_compile_options(sorting_central PRIVATE -Wall -Wextra -Wpedantic)
endif()
