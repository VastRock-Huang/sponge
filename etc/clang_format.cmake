if (NOT CLANG_FORMAT)
    if (DEFINED ENV{CLANG_FORMAT})
        set (CLANG_FORMAT_TMP $ENV{CLANG_FORMAT})
    else (NOT DEFINED ENV{CLANG_FORMAT})
        set (CLANG_FORMAT_TMP clang-format-6.0)
    endif (DEFINED ENV{CLANG_FORMAT})
    # figure out which version of clang-format we're using
    # execute_process(COMMAND <cmd1> [<arguments>]
    #                [RESULT_VARIABLE <variable>]
    #                [OUTPUT_VARIABLE <variable>])
    # --Execute one or more child processes.
    # `RESULT_VARIABLE`: an integer return code contain the result of last child process.
    # `OUTPUT_VARIABLE`: the contents of the standard output
    execute_process (COMMAND ${CLANG_FORMAT_TMP} --version
            RESULT_VARIABLE CLANG_FORMAT_RESULT
            OUTPUT_VARIABLE CLANG_FORMAT_VERSION)
    if (${CLANG_FORMAT_RESULT} EQUAL 0)
        # string(REGEX MATCH <match-regex> <out-var> <input>...)
        # --Match the <match-regex> once and store the match in the <out_var>.
        string (REGEX MATCH "version [0-9]" CLANG_FORMAT_VERSION ${CLANG_FORMAT_VERSION})
        message (STATUS "Found clang-format " ${CLANG_FORMAT_VERSION})
        set(CLANG_FORMAT ${CLANG_FORMAT_TMP} CACHE STRING "clang-format executable name")
    endif (${CLANG_FORMAT_RESULT} EQUAL 0)
endif (NOT CLANG_FORMAT)

if (DEFINED CLANG_FORMAT)
    # file(GLOB_RECURSE <variable> [<globbing-expressions>...])
    # --Generate a list of files that match the <globbing-expressions>
    #and store it into the <variable>
    file (GLOB_RECURSE ALL_CC_FILES *.cc)
    file (GLOB_RECURSE ALL_HH_FILES *.hh)
    add_custom_target (format ${CLANG_FORMAT} -i ${ALL_CC_FILES} ${ALL_HH_FILES} COMMENT "Formatted all source files.")
else (NOT DEFINED CLANG_FORMAT)
    add_custom_target (format echo "Could not find clang-format. Please install and re-run cmake")
endif (DEFINED CLANG_FORMAT)
