add_executable(libmobilesacn_test
    CrowLogHandlerTest.cpp
    EtcPalLogHandlerTest.cpp
    )

target_link_libraries(libmobilesacn_test PRIVATE libmobilesacn GTest::gtest_main
    mobilesacnproto
    crow::crow
    sACN
    )
include(GoogleTest)
gtest_discover_tests(libmobilesacn_test)
