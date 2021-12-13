# find_library (<VAR> name [path1 path2 ...]) -- find a library `name` and store as the variable `var`
find_library (LIBPCAP pcap)
find_library (LIBPTHREAD pthread)
# macro definition, (macro invocation is case-insensitive)
macro (add_sponge_exec exec_name)
    # add_executable(<name> [source1] [source2 ...])
    # --Add an executable to the project using the specified source files.
    add_executable ("${exec_name}" "${exec_name}.cc")
    # target_link_libraries(<target>... <item>...)
    # --specify libraries or flags to use when linking a given target.
    #It should use after add_executable()
    # ${ARGN}: holds the list of arguments past the last expected argument
    target_link_libraries ("${exec_name}" ${ARGN} sponge ${LIBPTHREAD})
endmacro (add_sponge_exec)
