add_executable(${PROJECT_NAME}
    main.cpp
    )

find_package(lyra REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
    libmobilesacn
    bfg::lyra
    )

#add_dependencies(${PROJECT_NAME} webui_build)
