# Generate the web interface
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(_NODE_NPM_NAME npm.cmd)
else ()
    set(_NODE_NPM_NAME npm)
endif ()
find_program(_NODE_NPM ${_NODE_NPM_NAME} REQUIRED)
message(STATUS "Installing webui dependencies")
execute_process(
    COMMAND "${_NODE_NPM}" install
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND_ERROR_IS_FATAL ANY
    ECHO_OUTPUT_VARIABLE ECHO_ERROR_VARIABLE
)
add_custom_target(webui_build
    COMMAND "${_NODE_NPM}" run build
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Building web ui..."
    )
add_dependencies(webui_build mobilesacnproto)

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_custom_command(TARGET webui_build POST_BUILD
            COMMAND "${CMAKE_COMMAND}"
            -E copy_directory "${PROJECT_SOURCE_DIR}/mobile_sacn_webui/build/"
            \"$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Resources/web\"
            )
else()
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/mobile_sacn_webui/build/ DESTINATION ${PROJECT_WEBUI_PATH})
endif ()

if (BUILD_TESTING)
    # Determine what tests are available.
    execute_process(
        COMMAND "${_NODE_NPM}" run test --silent -- --list-tests --watchAll=false
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE _JEST_TESTS
        COMMAND_ERROR_IS_FATAL ANY
    )
    foreach (_JEST_TEST_PATH "${_JEST_TESTS}")
        string(STRIP "${_JEST_TEST_PATH}" _JEST_TEST_PATH)
        if (NOT _JEST_TEST_PATH)
            # Empty line.
            continue()
        endif ()

        file(RELATIVE_PATH _JEST_TEST_REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}" "${_JEST_TEST_PATH}")
        add_test(
            NAME "${_JEST_TEST_REL_PATH}"
            COMMAND "${_NODE_NPM}" run test --silent -- --watchAll=false --runTestsByPath "${_JEST_TEST_PATH}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        )
    endforeach ()
endif ()
