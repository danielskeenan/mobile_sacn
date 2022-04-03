# Generate the web interface
find_program(_NODE_NPM npm REQUIRED)
add_custom_target(webui_node_modules
    COMMAND ${_NODE_NPM} install
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Installing npm modules..."
    )
add_custom_target(webui_build
    COMMAND ${_NODE_NPM} run build
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building web ui..."
    )
add_dependencies(webui_build webui_node_modules mobilesacnproto)

install(TARGETS ${PROJECT_NAME} RUNTIME)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/mobile_sacn_webui/build/ DESTINATION ${PROJECT_WEBUI_PATH})
