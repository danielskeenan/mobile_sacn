# Build the license files as HTML, RTF, and plain text
find_program(PANDOC_PATH NAMES pandoc REQUIRED)

set(LICENSE_SOURCE_FILE "${PROJECT_SOURCE_DIR}/resources/EULA.md")
list(APPEND CMAKE_CONFIGURE_DEPENDS ${LICENSE_SOURCE_FILE})
message(STATUS "Creating license files")
execute_process(COMMAND ${PANDOC_PATH}
    "--standalone" "--self-contained" "--from" "commonmark" "--to" "html" "--metadata" "title=End-User License Agreement/Privacy Policy" "${LICENSE_SOURCE_FILE}"
    OUTPUT_FILE "${PROJECT_BINARY_DIR}/LICENSE.html"
    ENCODING "UTF8"
    COMMAND_ERROR_IS_FATAL ANY
    )
execute_process(COMMAND ${PANDOC_PATH}
    "--standalone" "--self-contained" "--from" "commonmark" "--to" "rtf" "${LICENSE_SOURCE_FILE}"
    OUTPUT_FILE "${PROJECT_BINARY_DIR}/LICENSE.rtf"
    ENCODING "UTF8"
    COMMAND_ERROR_IS_FATAL ANY
    )
