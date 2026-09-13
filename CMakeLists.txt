cmake_minimum_required(VERSION 3.16)
project(ProcessMonitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Collect all C++ source files under src/ into a list. The CONFIGURE_DEPENDS
# option causes CMake to re-run the glob when files change.
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

add_executable(proc-mon ${PROJECT_SOURCES})

target_link_libraries(proc-mon PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

# Turn on common warning flags to help catch issues during compilation.
target_compile_options(proc-mon PRIVATE -Wall -Wextra -Wpedantic)

# ============================================================================
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Defaulting to Debug build type since no build type was specified.")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

# Check if the user explicitly requested a Debug build via -DCMAKE_BUILD_TYPE=Debug
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug build detected. Adding full debug symbols.")

    target_compile_definitions(proc-mon PRIVATE DEBUG)
    # -g adds debug symbols; -O0 disables optimizations so gdb can step accurately
    target_compile_options(proc-mon PRIVATE -g -O0)
else()
    message(STATUS "Standard build detected.")
endif()
# ============================================================================

# Copy config file to .config location
file(REAL_PATH "~" HOME_DIR EXPAND_TILDE)
set(CONFIG_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.config/")
set(CONFIG_DEST_DIR "${HOME_DIR}/.config/proc-mon/")

add_custom_target(CONFIG_FILES ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CONFIG_SRC_DIR}" "${CONFIG_DEST_DIR}"
    COMMENT "Copying config files to ${CONFIG_DEST_DIR} ..."
)

add_dependencies(CONFIG_FILES proc-mon)
