add_executable(${PROJECT_NAME}
    main.cpp
    )

find_package(lyra REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
    libmobilesacn
    bfg::lyra
    )

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION ".")
else ()
    install(TARGETS ${PROJECT_NAME} RUNTIME)
endif ()

add_dependencies(${PROJECT_NAME} webui_build)
