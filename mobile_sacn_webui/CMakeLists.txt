# Generate the web interface
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(_NODE_NPM_NAME npm.cmd)
else ()
    set(_NODE_NPM_NAME npm)
endif ()
find_program(_NODE_NPM ${_NODE_NPM_NAME} REQUIRED)
include(${PROJECT_SOURCE_DIR}/cmake/hashfiles.cmake)
hashfiles(_PACKAGE_LOCK_JSON_HASH package-lock.json)
if (PACKAGE_LOCK_JSON_HASH STREQUAL _PACKAGE_LOCK_JSON_HASH)
    message(STATUS "package-lock.json hasn't changed; skipping installation.")
else ()
    message(STATUS "Installing webui dependencies")
    execute_process(
        COMMAND "${_NODE_NPM}" install
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMAND_ERROR_IS_FATAL ANY
        ECHO_OUTPUT_VARIABLE ECHO_ERROR_VARIABLE
    )
    set(PACKAGE_LOCK_JSON_HASH "${_PACKAGE_LOCK_JSON_HASH}" CACHE INTERNAL "package-lock.json SHA256 hash")
endif ()
file(GLOB_RECURSE WEBUI_SOURCES CONFIGURE_DEPENDS
    public/* src/* config-overrides.js package-lock.json package.json tsconfig.json
    )
add_custom_command(OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/build/index.html"
    COMMAND "${_NODE_NPM}" run build
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Building web ui..."
    DEPENDS ${WEBUI_SOURCES}
    BYPRODUCTS "${CMAKE_CURRENT_SOURCE_DIR}/build"
    )
add_custom_target(webui_build DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/build/index.html")
add_dependencies(webui_build mobilesacnproto)

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_custom_command(TARGET webui_build POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
        -E copy_directory "${PROJECT_SOURCE_DIR}/mobile_sacn_webui/build/"
        \"$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/${PROJECT_WEBUI_PATH}\"
        )
else ()
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/mobile_sacn_webui/build/" DESTINATION "${PROJECT_WEBUI_PATH}")
endif ()

if (BUILD_TESTING)
    # Determine what tests are available.
    execute_process(
        COMMAND "${_NODE_NPM}" run test --silent -- --list-tests --watchAll=false
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE _JEST_TESTS
        COMMAND_ERROR_IS_FATAL ANY
    )
    # Transform test list into List.
    string(REPLACE "\n" ";" _JEST_TESTS "${_JEST_TESTS}")
    foreach (_JEST_TEST_PATH ${_JEST_TESTS})
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
