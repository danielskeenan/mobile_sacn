find_package(flatbuffers REQUIRED)

set(MESSAGE_SOURCES
    Address.fbs
    ChanCheck.fbs
    Level.fbs
    LevelBuffer.fbs
    PerAddressPriority.fbs
    Priority.fbs
    ReceiveLevelsReq.fbs
    ReceiveLevelsResp.fbs
    Transmit.fbs
    TransmitLevels.fbs
    Universe.fbs
)

# Set where generated files go.
# C++
set(MESSAGE_CPP_HEADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/cpp/mobilesacn_messages")
file(MAKE_DIRECTORY "${MESSAGE_CPP_HEADER_DIR}")
cmake_path(NATIVE_PATH MESSAGE_CPP_HEADER_DIR NORMALIZE MESSAGE_CPP_HEADER_DIR)
# Typescript
set(MESSAGE_TS_DIR "${PROJECT_SOURCE_DIR}/mobile_sacn_webui/src/messages")
file(MAKE_DIRECTORY "${MESSAGE_TS_DIR}")
cmake_path(NATIVE_PATH MESSAGE_TS_DIR NORMALIZE MESSAGE_TS_DIR)
# Needed because flatc creates a namespaced directory structure we don't want.
set(MESSAGE_TS_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/ts")
file(MAKE_DIRECTORY "${MESSAGE_TS_BUILD_DIR}")
cmake_path(NATIVE_PATH MESSAGE_TS_BUILD_DIR NORMALIZE MESSAGE_TS_BUILD_DIR)

# Link with this target to use messages.
add_library(mobile_sacn_messages INTERFACE)
target_include_directories(mobile_sacn_messages INTERFACE "${MESSAGE_CPP_HEADER_DIR}/..")
target_link_libraries(mobile_sacn_messages INTERFACE flatbuffers::flatbuffers)

# C++
foreach (FBS ${MESSAGE_SOURCES})
    cmake_path(GET FBS STEM LAST_ONLY FBS_NAME)
    set(MESSAGE_CPP_HEADER "${MESSAGE_CPP_HEADER_DIR}/${FBS_NAME}.h")
    add_custom_command(
        OUTPUT "${MESSAGE_CPP_HEADER}"
        COMMAND flatbuffers::flatc --cpp -o "${MESSAGE_CPP_HEADER_DIR}/" --scoped-enums --force-defaults --filename-suffix "" --cpp-field-case-style lower "${FBS}"
        MAIN_DEPENDENCY "${FBS}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        VERBATIM
    )
    target_sources(mobile_sacn_messages PRIVATE "${MESSAGE_CPP_HEADER}")
endforeach ()

# Typescript
# Can't use the same (cleaner...) approach as with CPP because the output file names are not known at configure-time.
add_custom_target(mobile_sacn_messages_ts
    COMMAND flatbuffers::flatc --ts -o "${MESSAGE_TS_BUILD_DIR}/" --force-defaults ${MESSAGE_SOURCES}
    COMMAND_EXPAND_LISTS
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    SOURCES ${MESSAGE_SOURCES}
    VERBATIM
)
add_dependencies(mobile_sacn_messages mobile_sacn_messages_ts)
# Needed because flatc creates a namespaced directory structure we don't want.
# Correct import paths and move to webui src directory.
add_executable(flatc_ts_fixer flatc_ts_fixer.cpp)
add_custom_command(TARGET mobile_sacn_messages_ts POST_BUILD
    COMMAND flatc_ts_fixer "${MESSAGE_TS_BUILD_DIR}/mobilesacn/message" "${MESSAGE_TS_DIR}"
    COMMENT "Fixing imports in generated Flatbuffers Typescript files..."
    VERBATIM
)
set_property(TARGET mobile_sacn_messages_ts APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${MESSAGE_TS_BUILD_DIR}" "${MESSAGE_TS_DIR}")
