function(deployqt)
    cmake_parse_arguments(deployqt " " "TARGET" "" ${ARGN})

    # Validate arguments.
    set(REQUIRED_ARGS TARGET)
    foreach (ARG IN ITEMS ${REQUIRED_ARGS})
        if (NOT DEFINED "deployqt_${ARG}")
            message(FATAL_ERROR "The argument ${ARG} is required.")
        endif ()
    endforeach ()
    if (NOT TARGET ${deployqt_TARGET})
        message(FATAL_ERROR "The target ${deployqt_TARGET} does not exist.")
    endif ()

    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        # Run windeployqt immediately after the build in the current binary dir.
        include(${PROJECT_SOURCE_DIR}/cmake/find_qt_tool.cmake)
        find_qt_tool(windeployqt REQUIRED)
        set(WINDEPLOYQT_COMMAND_ARGS
            --verbose 1
            --no-compiler-runtime
            --no-opengl-sw
            )
        if (NOT ${Qt_VERSION} EQUAL 6)
            # Qt6 doesn't use this option.
            list(APPEND WINDEPLOYQT_COMMAND_ARGS "--no-angle")
        endif ()
        list(JOIN "${WINDEPLOYQT_COMMAND_ARGS}" " " WINDEPLOYQT_COMMAND_ARGS)
        add_custom_command(TARGET ${deployqt_TARGET} POST_BUILD
            COMMAND "${Qt_WINDEPLOYQT_COMMAND}" ${WINDEPLOYQT_COMMAND_ARGS} "$<TARGET_FILE:${deployqt_TARGET}>"
            COMMENT "Running windeployqt for target ${deployqt_TARGET}"
            )

        # Run windeployqt upon installation.
        include(GNUInstallDirs)
        set(INSTALL_SCRIPT_PATH "${CMAKE_CURRENT_BINARY_DIR}/${deployqt_TARGET}.windeployqt.cmake")
        file(GENERATE OUTPUT "${INSTALL_SCRIPT_PATH}"
            CONTENT "message(STATUS \"Running windeployqt for target ${deployqt_TARGET}\")
            execute_process (
            COMMAND \"${Qt_WINDEPLOYQT_COMMAND}\" ${WINDEPLOYQT_COMMAND_ARGS} \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/$<TARGET_FILE_NAME:${deployqt_TARGET}>\"
            COMMAND_ERROR_IS_FATAL ANY
        )")
        install(SCRIPT "${INSTALL_SCRIPT_PATH}")
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin ")
        # Run macdeployqt immediately after the build in the current binary dir.
        include(${PROJECT_SOURCE_DIR}/cmake/find_qt_tool.cmake)
        find_qt_tool(macdeployqt REQUIRED)
        set(MACDEPLOYQT_COMMAND_ARGS
            --verbose 2
            --no-opengl-sw
            )
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${macdeployqt_PROG}" \"$<TARGET_BUNDLE_DIR:${deployqt_TARGET}>\"
            COMMENT "Running macdeployqt for target ${deployqt_TARGET}"
            )
    endif ()
endfunction()
