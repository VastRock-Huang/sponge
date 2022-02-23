cmake_minimum_required (VERSION 2.8.5)
cmake_policy (SET CMP0054 NEW)  # set CMake policy
project (Sponge)

# include(<file|module>
# --Load and run CMake code from a file or module.
include (etc/build_defs.cmake)
include (etc/build_type.cmake)  # set build type
include (etc/cflags.cmake)      # set compile flags

include (etc/doxygen.cmake)     # config Doxygen

include (etc/clang_format.cmake)    # config clang-format
include (etc/clang_tidy.cmake)      # config clang-tidy
include (etc/cppcheck.cmake)        # config cppcheck

# include_directories(dir1)
# --Add include directories to the build.
include_directories ("${PROJECT_SOURCE_DIR}/libsponge/util")
include_directories ("${PROJECT_SOURCE_DIR}/libsponge/tcp_helpers")
include_directories ("${PROJECT_SOURCE_DIR}/libsponge")

# add_subdirectory(source_dir)
# --Adds a subdirectory to the build. The source_dir specifies the
#directory in which the source CMakeLists.txt and code files are
#located.
add_subdirectory ("${PROJECT_SOURCE_DIR}/libsponge")    # add sponge library

add_subdirectory ("${PROJECT_SOURCE_DIR}/apps") # add webget exe

add_subdirectory ("${PROJECT_SOURCE_DIR}/tests")    # add test files

add_subdirectory ("${PROJECT_SOURCE_DIR}/doctests") # add doc test files

include (etc/tests.cmake)
