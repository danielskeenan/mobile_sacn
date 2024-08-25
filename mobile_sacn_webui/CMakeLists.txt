set(WEBUI_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/dist")
# The config assumes a relative path and will explode directories in the source dir if it is not given one.
cmake_path(RELATIVE_PATH WEBUI_BUILD_DIR BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE WEBUI_BUILD_DIR_REL)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/vite.config.ts.in" "${CMAKE_CURRENT_SOURCE_DIR}/vite.config.ts" @ONLY)

# Ensure favicon is correct.
file(COPY_FILE "${PROJECT_SOURCE_DIR}/resources/logo.svg" "${CMAKE_CURRENT_SOURCE_DIR}/public/mobile_sacn.svg" ONLY_IF_DIFFERENT)
file(COPY_FILE "${PROJECT_SOURCE_DIR}/resources/logo.svg" "${CMAKE_CURRENT_SOURCE_DIR}/src/assets/mobile_sacn.svg" ONLY_IF_DIFFERENT)

# Install Javascript deps
file(GLOB_RECURSE WEBUI_SOURCES LIST_DIRECTORIES false CONFIGURE_DEPENDS
    public/*
    src/*
    index.html
    *.json
    *.ts
)
find_program(npm_PROG npm DOC "Path to NPM (https://www.npmjs.com)" REQUIRED)
message(STATUS "Installing Web UI dependencies...")
execute_process(
    COMMAND "${npm_PROG}" install
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND_ERROR_IS_FATAL ANY
)
# Build site
add_custom_target(mobile_sacn_webui
    COMMENT "Building Web UI..."
    COMMAND "${npm_PROG}" run build
    SOURCES ${WEBUI_SOURCES}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS mobile_sacn_messages_ts
    VERBATIM
)
# Ensure make clean does what it should.
set_property(TARGET mobile_sacn_webui APPEND PROPERTY ADDITIONAL_CLEAN_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/node_modules"
    "${WEBUI_BUILD_DIR}"
)

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # Need extra logic to ensure it ends up inside the app bundle.
    add_custom_command(TARGET mobile_sacn_webui POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
        -E copy_directory "${WEBUI_BUILD_DIR}/"
        \"$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/${PROJECT_WEBUI_PATH}\"
    )
else ()
    install(DIRECTORY "${WEBUI_BUILD_DIR}/" DESTINATION "${PROJECT_WEBUI_PATH}")
endif ()
