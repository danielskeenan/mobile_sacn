if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_sources(${PROJECT_NAME} PRIVATE updater_win.cpp)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # TODO: Mac updater
    target_sources(${PROJECT_NAME} PRIVATE updater_none.cpp)
else ()
    target_sources(${PROJECT_NAME} PRIVATE updater_none.cpp)
endif ()
