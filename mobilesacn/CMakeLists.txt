configure_file(mobilesacn_config.h.in ${PROJECT_BINARY_DIR}/include/mobilesacn_config.h)
include_directories(${PROJECT_BINARY_DIR}/include)

add_subdirectory(libmobilesacn)
add_subdirectory(mobilesacn)
