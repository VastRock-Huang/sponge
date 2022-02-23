set (default_build_type "Release")
# set the `CMAKE_BUILD_TYPE_SHADOW`
if (NOT (CMAKE_BUILD_TYPE_SHADOW STREQUAL CMAKE_BUILD_TYPE))    # STREQUAL is used to compare two strings
    if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        message (STATUS "Setting build type to '${default_build_type}'")
        # set(<variable> <value>... CACHE <type> <docstring> [FORCE])
        # `CACHE`: set a cache entry
        # `FORCE`: overwrite existing entries
        set (CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
    else ()
        message (STATUS "Building in ${CMAKE_BUILD_TYPE} mode as requested.")
    endif ()
    set (CMAKE_BUILD_TYPE_SHADOW ${CMAKE_BUILD_TYPE} CACHE STRING "used to detect changes in build type" FORCE)
endif ()

message (STATUS "  NOTE: You can choose a build type by calling cmake with one of:")
message (STATUS "    -DCMAKE_BUILD_TYPE=Release   -- full optimizations")
message (STATUS "    -DCMAKE_BUILD_TYPE=Debug     -- better debugging experience in gdb")
message (STATUS "    -DCMAKE_BUILD_TYPE=RelASan   -- full optimizations plus address and undefined-behavior sanitizers")
message (STATUS "    -DCMAKE_BUILD_TYPE=DebugASan -- debug plus sanitizers")
