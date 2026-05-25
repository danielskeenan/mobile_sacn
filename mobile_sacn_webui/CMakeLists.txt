set(WEBUI_SOURCES
        i18next.config.ts
        index.html
        package-lock.json
        public/android-chrome-192x192.png
        public/android-chrome-512x512.png
        public/apple-touch-icon.png
        public/favicon-16x16.png
        public/favicon-32x32.png
        public/favicon.ico
        public/locales/en/channelCheck.json
        public/locales/en/common.json
        public/locales/en/receiveLevels.json
        public/locales/en/transmitLevels.json
        public/mobile_sacn.svg
        public/mobile_sacn_maskable.svg
        public/site.webmanifest
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
    file(COPY_FILE "${PROJECT_SOURCE_DIR}/resources/logo_maskable.svg" "${CMAKE_CURRENT_SOURCE_DIR}/public/mobile_sacn_maskable.svg" ONLY_IF_DIFFERENT)
    file(COPY_FILE "${PROJECT_SOURCE_DIR}/resources/logo.ico" "${CMAKE_CURRENT_SOURCE_DIR}/public/favicon.ico" ONLY_IF_DIFFERENT)
    file(COPY_FILE "${PROJECT_SOURCE_DIR}/resources/logo.svg" "${CMAKE_CURRENT_SOURCE_DIR}/src/assets/mobile_sacn.svg" ONLY_IF_DIFFERENT)

    # Build site
    add_custom_command(OUTPUT "${WEBUI_BUILD_DIR}/index.html"
            COMMENT "Building Web UI..."
            COMMAND "${CMAKE_COMMAND}" -E env WEBUI_BUILD_DIR_REL=${WEBUI_BUILD_DIR_REL} "${npm_PROG}" run build
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            DEPENDS ${WEBUI_SOURCES} mobile_sacn_messages_ts
            VERBATIM
    )
    add_custom_target(mobile_sacn_webui DEPENDS "${WEBUI_BUILD_DIR}/index.html")
    # Ensure make clean does what it should.
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} APPEND PROPERTY ADDITIONAL_CLEAN_FILES
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
endif ()
