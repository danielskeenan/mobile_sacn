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
if (TARGET webui_build)
    sphinx_build(doc_webui html "${CMAKE_CURRENT_BINARY_DIR}/webui/index.html")
    if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        add_custom_command(TARGET doc_webui POST_BUILD
            COMMAND "${CMAKE_COMMAND}"
            -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/webui/"
            \"$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Resources/web/doc\"
            )
    else ()
        install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/webui/" DESTINATION "${PROJECT_WEBUI_PATH}/doc")
    endif ()
endif ()
if (Qt_VERSION)
    if (Qt_VERSION EQUAL 5)
        find_package(Qt${Qt_VERSION} COMPONENTS Help REQUIRED)
    else ()
        find_package(Qt${Qt_VERSION} COMPONENTS Help Tools REQUIRED)
    endif ()
    sphinx_build(doc_qthelp qthelp "${CMAKE_CURRENT_BINARY_DIR}/qthelp/${PROJECT_NAME}.qhcp")
    # Need to add a few more options to the qhcp file.
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/qthelp/${PROJECT_NAME}_configured.qhcp"
        COMMAND Python3::Interpreter "${PROJECT_SOURCE_DIR}/scripts/qhcp.py"
        "${CMAKE_CURRENT_BINARY_DIR}/qthelp/${PROJECT_NAME}.qhcp"
        "${CMAKE_CURRENT_BINARY_DIR}/qthelp/${PROJECT_NAME}_configured.qhcp"
        DEPENDS "${PROJECT_SOURCE_DIR}/scripts/qhcp.py" "${CMAKE_CURRENT_BINARY_DIR}/qthelp/${PROJECT_NAME}.qhcp"
        COMMENT "Adding Qt Assistant configuration to ${CMAKE_CURRENT_BINARY_DIR}/qthelp/${PROJECT_NAME}.qhcp"
        )
    file(COPY "${PROJECT_SOURCE_DIR}/resources/logo.svg" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/qthelp/")
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/qthelp/${PROJECT_NAME}.qhc"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qthelp/${PROJECT_NAME}_configured.qhcp"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qthelp"
        COMMAND Qt${Qt_VERSION}::qhelpgenerator
        "${CMAKE_CURRENT_BINARY_DIR}/qthelp/${PROJECT_NAME}_configured.qhcp"
        -o "${CMAKE_CURRENT_BINARY_DIR}/qthelp/${PROJECT_NAME}.qhc"
        )
    add_custom_target(doc_qthelp_qhc DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qthelp/${PROJECT_NAME}.qhc")

    # Help installation
    if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        # This is installed as part of the main executable build.
    else ()
        install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/qthelp/${PROJECT_NAME}.qhc"
            "${CMAKE_CURRENT_BINARY_DIR}/qthelp/${PROJECT_NAME}.qch"
            DESTINATION "${PROJECT_HELP_PATH}"
            )
        if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
            # Install assistant.
            set(ASSISTANT_PATH "$<TARGET_FILE_DIR:Qt${Qt_VERSION}::qhelpgenerator>/assistant${CMAKE_EXECUTABLE_SUFFIX}")
            install(FILES "${ASSISTANT_PATH}" TYPE BIN)
            include(GNUInstallDirs)
            find_windeployqt()
            if (${Qt_VERSION} EQUAL 6)
                # Qt6 doesn't use this option.
                set(_NO_ANGLE "")
            else ()
                set(_NO_ANGLE "--no-angle")
            endif ()
            install(CODE
                "
        execute_process(
            COMMAND \"${CMAKE_COMMAND}\" -E
                env \"PATH=$<TARGET_FILE_DIR:Qt${Qt_VERSION}::qmake>\" \"${WINDEPLOYQT_EXECUTABLE}\"
                    --no-compiler-runtime ${_NO_ANGLE}
                    --no-opengl-sw
                    --dir \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\"
                    \"${ASSISTANT_PATH}\"
            COMMAND_ERROR_IS_FATAL ANY
        )
        "
                )
        endif ()
    endif ()
endif ()
