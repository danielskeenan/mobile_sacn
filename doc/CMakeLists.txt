set(DOC_SOURCES
        3rd_party.md
        _static/bootstrap-icons.min.css
        _static/fonts/bootstrap-icons.woff
        _static/fonts/bootstrap-icons.woff2
        _static/msacn-doc.css
        conf.py
        getting-started.md
        img/screenshots/base_started_dark.png
        img/screenshots/base_started_light.png
        index.md
        privacy_policy.md
        requirements.txt
        usage/chancheck/index.md
        usage/chancheck/screenshots/addr_notransmit_dark.png
        usage/chancheck/screenshots/addr_notransmit_light.png
        usage/chancheck/screenshots/addr_transmit_dark.png
        usage/chancheck/screenshots/addr_transmit_light.png
        usage/chancheck/screenshots/chancheck_dark.png
        usage/chancheck/screenshots/chancheck_light.png
        usage/chancheck/screenshots/config_dark.png
        usage/chancheck/screenshots/config_light.png
        usage/chancheck/screenshots/level_dark.png
        usage/chancheck/screenshots/level_light.png
        usage/control/index.md
        usage/control/screenshots/config_dark.png
        usage/control/screenshots/config_light.png
        usage/control/screenshots/control_faders_dark.png
        usage/control/screenshots/control_faders_light.png
        usage/control/screenshots/control_keypad_dark.png
        usage/control/screenshots/control_keypad_light.png
        usage/index.md
        usage/nav_screenshot_dark.png
        usage/nav_screenshot_light.png
        usage/view_levels/index.md
        usage/view_levels/screenshots/flickerfinder_list_dark.png
        usage/view_levels/screenshots/flickerfinder_list_light.png
        usage/view_levels/screenshots/univ_discovery_dark.png
        usage/view_levels/screenshots/univ_discovery_light.png
        usage/view_levels/screenshots/viewlevels_grid_dark.png
        usage/view_levels/screenshots/viewlevels_grid_light.png
        usage/view_levels/screenshots/viewlevels_list_dark.png
        usage/view_levels/screenshots/viewlevels_list_light.png
)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Create venv
set(DOC_VENV_PATH "${CMAKE_CURRENT_BINARY_DIR}/.venv")
if (NOT IS_DIRECTORY "${DOC_VENV_PATH}")
    message(STATUS "Creating Python venv for documentation in ${DOC_VENV_PATH}...")
    execute_process(
            COMMAND "${Python3_EXECUTABLE}" -m venv "${DOC_VENV_PATH}"
            COMMAND_ERROR_IS_FATAL ANY
    )
endif ()

# See https://discourse.cmake.org/t/possible-to-create-a-python-virtual-env-from-cmake-and-then-find-it-with-findpython3/1132
set(ENV{VIRTUAL_ENV} "${DOC_VENV_PATH}")
set(Python3_FIND_VIRTUALENV ONLY)
unset(Python3_EXECUTABLE)
# Need to unset these env vars as otherwise GitHub Actions will force the system python, not the venv.
unset(ENV{Python_ROOT_DIR})
unset(ENV{Python3_ROOT_DIR})
find_package(Python3 COMPONENTS Interpreter REQUIRED)
execute_process(
        COMMAND "${Python3_EXECUTABLE}" -m pip install -U -r "${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt"
        COMMAND_ERROR_IS_FATAL ANY
)
set_directory_properties(PROPERTIES CMAKE_CONFIGURE_DEPENDS requirements.txt)

# Create build info to send to Sphinx
set(SPHINX_BUILD_INFO "{ }")
string(JSON SPHINX_BUILD_INFO SET "${SPHINX_BUILD_INFO}" "name" "\"${PROJECT_NAME}\"")
string(JSON SPHINX_BUILD_INFO SET "${SPHINX_BUILD_INFO}" "display_name" "\"${PROJECT_DISPLAY_NAME}\"")
string(JSON SPHINX_BUILD_INFO SET "${SPHINX_BUILD_INFO}" "author" "\"${PROJECT_AUTHOR}\"")
string(JSON SPHINX_BUILD_INFO SET "${SPHINX_BUILD_INFO}" "version" "\"${PROJECT_VERSION}\"")
file(WRITE "${CMAKE_CURRENT_LIST_DIR}/build_info.json" "${SPHINX_BUILD_INFO}")

# Generate doc
find_program(SPHINX_BUILD sphinx-build
        PATHS "${DOC_VENV_PATH}"
        PATH_SUFFIXES bin Scripts
        NO_DEFAULT_PATH
        REQUIRED
)

# 3rd party license page.
file(GLOB LOCAL_SBOMS LIST_DIRECTORIES false CONFIGURE_DEPENDS
        licenses/*.spdx.json
)
add_custom_command(OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/3rd_party.md"
        COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/3rd_party.py"
        -o "${CMAKE_CURRENT_SOURCE_DIR}/3rd_party.md"
        --vcpkg_install_root "${VCPKG_INSTALLED_DIR}"
        --npm_project_root "${PROJECT_SOURCE_DIR}/mobile_sacn_webui"
        ${LOCAL_SBOMS}
        DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/3rd_party.py"
        "${PROJECT_SOURCE_DIR}/mobile_sacn_webui/package-lock.json"
        "${VCPKG_MANIFEST_DIR}/vcpkg.json"
        ${LOCAL_SBOMS}
        COMMAND_EXPAND_LISTS
)
list(APPEND DOC_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/3rd_party.md")
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html/index.html"
        COMMAND "${SPHINX_BUILD}" -M html "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Generating documentation (html)"
        DEPENDS ${DOC_SOURCES}
        VERBATIM
)
add_custom_target(${PROJECT_NAME}_doc DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/index.html")
set_property(TARGET ${PROJECT_NAME}_doc APPEND PROPERTY ADDITIONAL_CLEAN_FILES
        "${CMAKE_CURRENT_BINARY_DIR}/html"
)
install(
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/
        DESTINATION "${PROJECT_WEBUI_PATH}/doc"
)

# Archive for packaging
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_doc.zip"
        COMMAND "${CMAKE_COMMAND}" -E tar c
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_doc.zip"
        --format=zip
        "${CMAKE_CURRENT_BINARY_DIR}/html"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/index.html"
)
add_custom_target(${PROJECT_NAME}_doc_archive DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_doc.zip")
