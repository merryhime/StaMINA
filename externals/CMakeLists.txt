# For libraries that already come with a CMakeLists file,
# simply add the directory to that file as a subdirectory
# to have CMake automatically recognize them.

# catch

add_library(catch INTERFACE)
target_include_directories(catch INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/catch>)

# fmt

# fmtlib formatting library
add_subdirectory(fmt)
