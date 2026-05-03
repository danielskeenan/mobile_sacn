set(WEBUI_SOURCES
        i18next.config.ts
        index.html
        package-lock.json
        public/locales/en/channelCheck.json
        public/locales/en/common.json
        public/locales/en/receiveLevels.json
        public/locales/en/transmitLevels.json
        src/App.scss
        src/App.tsx
        src/assets/mobile_sacn.svg
        src/assets/styles/_bootstrap.scss
        src/assets/styles/_colors.scss
        src/assets/styles/_content.scss
        src/common/AppContext.tsx
        src/common/bigIntAbs.ts
        src/common/cidColor.ts
        src/common/clamp.ts
        src/common/components/ConnectButton.scss
        src/common/components/ConnectButton.tsx
        src/common/components/Connecting.tsx
        src/common/components/LevelBar.scss
        src/common/components/LevelBar.tsx
        src/common/components/LevelDisplay.scss
        src/common/components/LevelDisplay.tsx
        src/common/components/Loading.tsx
        src/common/constants.ts
        src/common/generate.ts
        src/common/getBootstrapColor.ts
        src/common/handleInputChange.ts
        src/common/iota.ts
        src/common/levelDisplay.ts
        src/common/translator.ts
        src/common/unique.ts
        src/errors/404.tsx
        src/index.tsx
        src/links.ts
        src/pages/front/HomePage.tsx
        src/pages/receive/levels/ReceiveLevelsPage.scss
        src/pages/receive/levels/ReceiveLevelsPage.tsx
        src/pages/receive/levels/ReceiveLevelsTitle.tsx
        src/pages/transmit/ChannelCheckPage.scss
        src/pages/transmit/ChannelCheckPage.tsx
        src/pages/transmit/ChannelCheckTitle.tsx
        src/pages/transmit/TransmitConfig.scss
        src/pages/transmit/TransmitConfig.tsx
        src/pages/transmit/TransmitLevelsPage.scss
        src/pages/transmit/TransmitLevelsPage.tsx
        src/pages/transmit/TransmitLevelsTitle.tsx
        src/pages/transmit/cmdline.ts
        src/routes.ts
        tsconfig.app.json
        tsconfig.json
        tsconfig.node.json
        vite.config.ts
)

# Install Javascript deps
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(_NPM_NAME npm.cmd)
else ()
    set(_NPM_NAME npm)
endif ()
find_program(npm_PROG ${_NPM_NAME} DOC "Path to NPM (https://www.npmjs.com)" REQUIRED)
set(NODE_MODULES_LOCKFILE "${CMAKE_CURRENT_SOURCE_DIR}/node_modules.lock")
# Only run npm install if the package lock file has changed.
file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/package-lock.json" CURRENT_PACKAGE_LOCK_HASH)
if (NOT "${WEBUI_PACKAGE_LOCK_HASH}" STREQUAL "${CURRENT_PACKAGE_LOCK_HASH}" OR NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/node_modules")
    if (EXISTS "${NODE_MODULES_LOCKFILE}")
        message(STATUS "Waiting for lock on node_modules")
        while (EXISTS "${NODE_MODULES_LOCKFILE}")
            execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 1.0)
        endwhile ()
    endif ()
    file(TOUCH "${NODE_MODULES_LOCKFILE}")
    message(STATUS "Installing Web UI dependencies...")
    execute_process(
            COMMAND "${npm_PROG}" install
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMAND_ERROR_IS_FATAL ANY
    )
    file(REMOVE "${NODE_MODULES_LOCKFILE}")
endif ()
set(WEBUI_PACKAGE_LOCK_HASH "${CURRENT_PACKAGE_LOCK_HASH}" CACHE INTERNAL "" FORCE)

# Because the NPM deps are also used for doc, need an extra check to see if we really want to build the web UI.
if (BUILD_EXEC)
    set(WEBUI_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/dist")
    # The config assumes a relative path and will explode directories in the source dir if it is not given one.
    # This is injected into the process environment when building.
    cmake_path(RELATIVE_PATH WEBUI_BUILD_DIR BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE WEBUI_BUILD_DIR_REL)

    # Ensure favicon is correct.
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/public")
    file(COPY_FILE "${PROJECT_SOURCE_DIR}/resources/logo.svg" "${CMAKE_CURRENT_SOURCE_DIR}/public/mobile_sacn.svg" ONLY_IF_DIFFERENT)
    file(COPY_FILE "${PROJECT_SOURCE_DIR}/resources/logo.svg" "${CMAKE_CURRENT_SOURCE_DIR}/src/assets/mobile_sacn.svg" ONLY_IF_DIFFERENT)

    # Build site
    add_custom_command(OUTPUT "${WEBUI_BUILD_DIR}/index.html"
            COMMENT "Building Web UI..."
            COMMAND "${CMAKE_COMMAND}" -E env WEBUI_BUILD_DIR_REL=${WEBUI_BUILD_DIR_REL} "${npm_PROG}" run build
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            DEPENDS ${WEBUI_SOURCES} mobile_sacn_messages_ts
            VERBATIM
    )
    # Ensure make clean does what it should.
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} APPEND PROPERTY ADDITIONAL_CLEAN_FILES
            "${WEBUI_BUILD_DIR}"
    )

    # Add as Qt resource.
    find_package(Qt6 COMPONENTS Core REQUIRED)
    add_executable(gen_webui_qrc gen_webui_qrc.cpp)
    target_link_libraries(gen_webui_qrc PRIVATE Qt::Core)
    if (NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
        qt_generate_deploy_app_script(
                TARGET gen_webui_qrc
                OUTPUT_SCRIPT DEPLOY_SCRIPT
                NO_UNSUPPORTED_PLATFORM_ERROR
        )
        add_custom_command(
                TARGET gen_webui_qrc POST_BUILD
                COMMAND "${CMAKE_COMMAND}" "-DCMAKE_INSTALL_PREFIX=$<TARGET_FILE_DIR:gen_webui_qrc>" -DQT_DEPLOY_BIN_DIR=.
                -DQT_DEPLOY_LIB_DIR=. -P "${DEPLOY_SCRIPT}"
                VERBATIM
        )
    endif ()
    set(WEBUI_QRC "${CMAKE_CURRENT_BINARY_DIR}/webui.qrc")
    add_custom_command(
            OUTPUT "${WEBUI_QRC}"
            COMMAND gen_webui_qrc "${WEBUI_BUILD_DIR}" "${WEBUI_QRC}"
            WORKING_DIRECTORY "${WEBUI_BUILD_DIR}"
            DEPENDS "${WEBUI_BUILD_DIR}/index.html" ${WEBUI_SOURCES}
            VERBATIM
    )
    add_library(webui_qrc STATIC "${WEBUI_QRC}")
    set_target_properties(webui_qrc PROPERTIES LINKER_LANGUAGE CXX)
    target_link_libraries(webui_qrc PRIVATE Qt::Core)
endif ()
