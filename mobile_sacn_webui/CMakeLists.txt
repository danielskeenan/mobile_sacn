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
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND_ERROR_IS_FATAL ANY
    ECHO_OUTPUT_VARIABLE ECHO_ERROR_VARIABLE
)
add_custom_target(webui_build
    COMMAND "${_NODE_NPM}" run build
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building web ui..."
    )
add_dependencies(webui_build mobilesacnproto)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/mobile_sacn_webui/build/ DESTINATION ${PROJECT_WEBUI_PATH})
