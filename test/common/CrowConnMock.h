/**
 * @file CrowConnMock.h
 *
 * @author Dan Keenan
 * @date 4/27/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_TEST_COMMON_CROWCONNMOCK_H_
#define MOBILE_SACN_TEST_COMMON_CROWCONNMOCK_H_

#include <gmock/gmock.h>
#include <crow/websocket.h>

namespace mobilesacn::testing {

/**
 * Mock for crow::websocket::connection.
 */
class CrowConnMock : public crow::websocket::connection {
 public:
  MOCK_METHOD(void, send_binary, (const std::string& msg), (override));
  MOCK_METHOD(void, send_text, (const std::string& msg), (override));
  MOCK_METHOD(void, send_ping, (const std::string& msg), (override));
  MOCK_METHOD(void, send_pong, (const std::string& msg), (override));
  MOCK_METHOD(void, close, (const std::string& msg ), (override));

  std::string get_remote_ip() override {
    return "127.0.0.1";
  }
};

}

#endif //MOBILE_SACN_TEST_COMMON_CROWCONNMOCK_H_
