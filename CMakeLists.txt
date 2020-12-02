cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(stamina CXX)

# Project options
option(STAMINA_WARNINGS_AS_ERRORS "Warnings as errors" ON)

# Default to a Release build
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
    message(STATUS "Defaulting to a Release build")
endif()

# Set hard requirements for C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warn on CMake API deprecations
set(CMAKE_WARN_DEPRECATED ON)

# Disable in-source builds
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(SEND_ERROR "In-source builds are not allowed.")
endif()

# Add the module directory to the list of paths
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")

# Compiler flags
if (MSVC)
    set(STAMINA_CXX_FLAGS
        /std:c++latest
        /experimental:external
        /external:W0
        /external:anglebrackets
        /W4
        /w44263 # Non-virtual member function hides base class virtual function
        /w44265 # Class has virtual functions, but destructor is not virtual
        /w44456 # Declaration of 'var' hides previous local declaration
        /w44457 # Declaration of 'var' hides function parameter
        /w44458 # Declaration of 'var' hides class member
        /w44459 # Declaration of 'var' hides global definition
        /w44946 # Reinterpret-cast between related types
        /wd4592 # Symbol will be dynamically initialized (implementation limitation)
        /permissive- # Stricter C++ standards conformance
        /MP
        /Zi
        /Zo
        /EHsc
        /Zc:externConstexpr # Allows external linkage for variables declared "extern constexpr", as the standard permits.
        /Zc:inline          # Omits inline functions from object-file output.
        /Zc:throwingNew     # Assumes new (without std::nothrow) never returns null.
        /volatile:iso       # Use strict standard-abiding volatile semantics
        /bigobj             # Increase number of sections in .obj files
        /DNOMINMAX)

    if (STAMINA_WARNINGS_AS_ERRORS)
        list(APPEND STAMINA_CXX_FLAGS /WX)
    endif()

    if (CMAKE_VS_PLATFORM_TOOLSET MATCHES "LLVM-vs[0-9]+")
        list(APPEND STAMINA_CXX_FLAGS
             -Qunused-arguments
             -Wno-missing-braces)
    endif()
else()
    set(STAMINA_CXX_FLAGS
        -Wall
        -Wextra
        -Wcast-qual
        -pedantic
        -pedantic-errors
        -Wfatal-errors
        -Wno-missing-braces)

    if (STAMINA_WARNINGS_AS_ERRORS)
        list(APPEND STAMINA_CXX_FLAGS -Werror)
    endif()
endif()

# Enable unit-testing
enable_testing(true)

# Pull in externals CMakeLists for libs where available
add_subdirectory(externals)

# Project files

add_library(common
    src/common/assert.cpp
    src/common/assert.hpp
    src/common/common_types.hpp
)
target_include_directories(common PUBLIC src)
target_compile_options(common PRIVATE ${STAMINA_CXX_FLAGS})
target_link_libraries(common PUBLIC fmt)

add_library(smasm-lib
    src/smasm/lexer.cpp
    src/smasm/lexer.hpp
    src/smasm/position.hpp
)
target_include_directories(smasm-lib PUBLIC src)
target_compile_options(smasm-lib PRIVATE ${STAMINA_CXX_FLAGS})
target_link_libraries(smasm-lib PUBLIC common fmt)

add_executable(smasm
    src/smasm/main.cpp
)
target_include_directories(smasm PUBLIC src)
target_compile_options(smasm PRIVATE ${STAMINA_CXX_FLAGS})
target_link_libraries(smasm PRIVATE common smasm-lib)

add_executable(stamina
    src/stamina/main.cpp
)
target_include_directories(stamina PUBLIC src)
target_compile_options(stamina PRIVATE ${STAMINA_CXX_FLAGS})
target_link_libraries(stamina PRIVATE common)

add_executable(stamina-tests
    src/smasm/lexer_tests.cpp
    src/tests/main.cpp
)
target_include_directories(stamina-tests PUBLIC src)
target_compile_options(stamina-tests PRIVATE ${STAMINA_CXX_FLAGS})
target_link_libraries(stamina-tests PRIVATE catch common smasm-lib)

include(CreateDirectoryGroups)
create_target_directory_groups(common)
create_target_directory_groups(smasm-lib)
create_target_directory_groups(smasm)
create_target_directory_groups(stamina)
create_target_directory_groups(stamina-tests)
