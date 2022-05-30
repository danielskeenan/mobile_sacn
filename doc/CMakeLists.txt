find_program(SPHINX_EXECUTABLE sphinx-build REQUIRED)
file(GLOB_RECURSE DOC_SOURCES CONFIGURE_DEPENDS ./*)

add_custom_target(doc SOURCES ${DOC_SOURCES})
function(sphinx_build TARGET_NAME BUILDER PRIMARY_OUTPUT)
    get_filename_component(OUTPUT_DIR "${PRIMARY_OUTPUT}" DIRECTORY)

    string(TIMESTAMP CURRENT_YEAR "%Y" UTC)
    add_custom_command(OUTPUT "${PRIMARY_OUTPUT}"
        COMMAND "${SPHINX_EXECUTABLE}"
        -b ${BUILDER}
        -j auto
        -W --keep-going
        -T
        -d "${CMAKE_CURRENT_BINARY_DIR}/doctrees_${BUILDER}"
        -D "project=${PROJECT_DISPLAY_NAME}"
        -D "author=${PROJECT_AUTHOR}"
        -D "version=${PROJECT_VERSION}"
        -D "release=${PROJECT_VERSION}"
        -D "copyright=${CURRENT_YEAR}, ${PROJECT_AUTHOR}"
        -D "qthelp_basename=${PROJECT_NAME}"
        -D "qthelp_namespace=org.dankeenan.${PROJECT_NAME}.${PROJECT_VERSION}"
        ${ARGN}
        "${CMAKE_CURRENT_SOURCE_DIR}" "${OUTPUT_DIR}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Building documentation (${BUILDER})..."
        DEPENDS ${DOC_SOURCES}
        BYPRODUCTS "${OUTPUT_DIR}"
        )

    add_custom_target(${TARGET_NAME} DEPENDS "${PRIMARY_OUTPUT}")
    add_dependencies(doc ${TARGET_NAME})
endfunction()

sphinx_build(doc_html html "${CMAKE_CURRENT_BINARY_DIR}/html/index.html")
sphinx_build(doc_webui html "${CMAKE_CURRENT_BINARY_DIR}/webui/index.html")
add_dependencies(doc_webui webui_build)
sphinx_build(doc_qthelp qthelp "${CMAKE_CURRENT_BINARY_DIR}/qthelp/${PROJECT_NAME}.qhcp")

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_custom_command(TARGET doc_webui POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
        -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/webui/"
        \"$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Resources/web/doc\"
        )
else ()
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/webui/" DESTINATION "${PROJECT_WEBUI_PATH}/doc")
endif ()
