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
add_custom_target(${PROJECT_NAME}_doc
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
