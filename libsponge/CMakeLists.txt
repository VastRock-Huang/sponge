file (GLOB LIB_SOURCES "*.cc" "util/*.cc" "tcp_helpers/*.cc")
# add_library(<name> [STATIC | SHARED | MODULE]
#            [<source>...])
# --Add a library to the project using the specified source files.
add_library (sponge STATIC ${LIB_SOURCES})
