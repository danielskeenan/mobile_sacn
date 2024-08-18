find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Create venv
message(STATUS "Creating Sphinx venv")
execute_process(COMMAND "${Python3_EXECUTABLE}" -m venv .venv
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND_ERROR_IS_FATAL ANY
)
find_program(SPHINX_PIP pip
    PATHS "${CMAKE_CURRENT_SOURCE_DIR}/.venv"
    PATH_SUFFIXES bin Scripts
    NO_DEFAULT_PATH
    REQUIRED
)
execute_process(COMMAND "${SPHINX_PIP}" install --upgrade -r requirements.txt
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND_ERROR_IS_FATAL ANY
)

# Create build info to send to Sphinx
set(SPHINX_BUILD_INFO "{ }")
string(JSON SPHINX_BUILD_INFO SET "${SPHINX_BUILD_INFO}" "name" "\"${PROJECT_NAME}\"")
string(JSON SPHINX_BUILD_INFO SET "${SPHINX_BUILD_INFO}" "display_name" "\"${PROJECT_DISPLAY_NAME}\"")
string(JSON SPHINX_BUILD_INFO SET "${SPHINX_BUILD_INFO}" "author" "\"${PROJECT_AUTHOR}\"")
string(JSON SPHINX_BUILD_INFO SET "${SPHINX_BUILD_INFO}" "version" "\"${PROJECT_VERSION}\"")
file(WRITE "${CMAKE_CURRENT_LIST_DIR}/build_info.json" "${SPHINX_BUILD_INFO}")

# Generate doc
find_program(SPHINX_BUILD sphinx-build
    PATHS "${CMAKE_CURRENT_SOURCE_DIR}/.venv"
    PATH_SUFFIXES bin Scripts
    NO_DEFAULT_PATH
    REQUIRED
)

# HTML Docs
add_custom_target(${PROJECT_NAME}_doc_html
    COMMAND "${SPHINX_BUILD}" -M html "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/html/index.html"
    COMMENT "Generating documentation (html)"
    VERBATIM
)
install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/
    DESTINATION "${PROJECT_HELP_PATH}"
)

# Qt Help (for embedding into the program)
#find_package(Qt6 COMPONENTS Help)
#if (Qt6_FOUND)
#    add_custom_target(${PROJECT_NAME}_doc_qthelp
#            COMMAND "${SPHINX_BUILD}" -M qthelp "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}"
#            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
#            BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/qthelp/${PROJECT_NAME}.qhcp"
#            COMMENT "Generating documentation (qthelp)"
#            VERBATIM
#    )
#
#    # Need to add a few more options to the qhcp file.
#    add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/doc/qthelp/${PROJECT_NAME}_configured.qhcp"
#            COMMAND Python3::Interpreter "${PROJECT_SOURCE_DIR}/scripts/qhcp.py"
#            "${PROJECT_BINARY_DIR}/doc/qthelp/${PROJECT_NAME}.qhcp"
#            "${PROJECT_BINARY_DIR}/doc/qthelp/${PROJECT_NAME}_configured.qhcp"
#            DEPENDS "${PROJECT_SOURCE_DIR}/scripts/qhcp.py" "${PROJECT_BINARY_DIR}/doc/qthelp/${PROJECT_NAME}.qhcp"
#            COMMENT "Adding Qt Assistant configuration to ${PROJECT_BINARY_DIR}/doc/qthelp/${PROJECT_NAME}.qhcp"
#    )
#    file(COPY "${PROJECT_SOURCE_DIR}/patchman.svg" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/qthelp/")
#    add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/doc/qthelp/${PROJECT_NAME}.qhc"
#            DEPENDS "${PROJECT_BINARY_DIR}/doc/qthelp/${PROJECT_NAME}_configured.qhcp"
#            WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/doc/qthelp"
#            COMMAND Qt::qhelpgenerator
#            "${PROJECT_BINARY_DIR}/doc/qthelp/${PROJECT_NAME}_configured.qhcp"
#            -o "${PROJECT_BINARY_DIR}/doc/qthelp/${PROJECT_NAME}.qhc"
#    )
#    add_custom_target(${PROJECT_NAME}_doc_qthelp_qhc DEPENDS "${PROJECT_BINARY_DIR}/doc/qthelp/${PROJECT_NAME}.qhc")
#    install(FILES
#            "${PROJECT_BINARY_DIR}/doc/qthelp/${PROJECT_NAME}.qhc"
#            "${PROJECT_BINARY_DIR}/doc/qthelp/${PROJECT_NAME}.qch"
#            DESTINATION "${PROJECT_HELP_PATH}"
#    )
#endif ()
