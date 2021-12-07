find_package (Doxygen)
if (DOXYGEN_FOUND)
    if (Doxygen_dot_FOUND)
        set (DOXYGEN_DOT_FOUND YES)
    else (NOT Doxygen_dot_FOUND)
        set (DOXYGEN_DOT_FOUND NO)
    endif (Doxygen_dot_FOUND)
    # configure_file(<input> <output> [@ONLY])
    # --Copies an <input> file to an <output> file,  and substitutes variable values referenced as
    # @VAR@ or ${VAR} in the input file content.
    # @ONLY: Restrict variable replacement to references of the form `@VAR@`.
    configure_file ("${PROJECT_SOURCE_DIR}/etc/Doxyfile.in" "${PROJECT_BINARY_DIR}/Doxyfile" @ONLY)
    # add_custom_target(Name [ALL] [command1 [args1...]]
    #                  [COMMAND command2 [args2...] ...]
    #                  [WORKING_DIRECTORY dir]
    #                  [COMMENT comment])
    # --Adds a target with the given name that executes the given commands.
    add_custom_target (doc "${DOXYGEN_EXECUTABLE}" "${PROJECT_BINARY_DIR}/Doxyfile"
                           WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
                           COMMENT "Generate docs using Doxygen" VERBATIM)
endif ()
