add_executable(libmobilesacn_test
    CrowLogHandlerTest.cpp
    EtcPalLogHandlerTest.cpp
    )

find_package(crow REQUIRED)
target_link_libraries(libmobilesacn_test PRIVATE libmobilesacn GTest::gtest_main GTest::gmock
    mobilesacnproto
    crow::crow
    sACN
    )

add_subdirectory(fx)
add_subdirectory(rpc)

include(GoogleTest)
gtest_discover_tests(libmobilesacn_test)
