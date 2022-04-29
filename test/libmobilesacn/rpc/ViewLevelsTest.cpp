/**
 * @file ViewLevelsTest.cpp
 *
 * @author Dan Keenan
 * @date 4/27/22
 * @copyright GNU GPLv3
 */

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <map>
#include "../../common/CrowConnMock.h"
#include "../../common/SacnTest.h"
#include "../../common/SpdLogNotifySink.h"
#include "proto/view_levels.pb.h"
#include "libmobilesacn/rpc/ViewLevels.h"
#include <sacn/cpp/source.h>

using ::testing::NiceMock;
using ::testing::AtLeast;

class ViewLevelsTest : public mobilesacn::testing::SacnTest {
};

TEST_F(ViewLevelsTest, ViewLevels) {
  // Setup the log observer.
  static const std::string log_msg = "New view_levels connection from 127.0.0.1";
  unsigned int log_count = 0;
  mobilesacn::testing::NotifySinkSt::OnLogCb log_cb = [&log_count](const spdlog::details::log_msg &msg) {
    EXPECT_LT(msg.level, spdlog::level::warn);
    EXPECT_EQ(std::string_view(msg.payload.begin(), msg.payload.end()), log_msg);
    ++log_count;
  };
  auto test_sink = std::make_shared<mobilesacn::testing::NotifySinkSt>(log_cb);
  spdlog::default_logger()->sinks() = {test_sink};

  // Setup.
  auto test_univ = 1;
  std::map<std::string, std::string> test_sources;
  mobilesacn::testing::DmxBuffer test_buf{0};
  std::array<std::string, DMX_ADDRESS_COUNT> test_winning_sources{};
  const auto test_cid = etcpal::Uuid::V4();
  const std::string test_source_name = "test_transmitter";
  const sacn::Source::Settings sacn_transmitter_settings(test_cid, test_source_name);
  sacn::Source sacn_transmitter;
  sacn_transmitter.Startup(sacn_transmitter_settings);
  mobilesacn::rpc::ViewLevelsReq req;
  // The View Levels RPC handler will always send an initial response upon connection before checking for levels.
  mobilesacn::rpc::ViewLevelsRes initial_res;

  // Initial state.
  req.set_universe(test_univ);
  initial_res.set_universe(test_univ);
  initial_res.clear_sources();
  initial_res.clear_levels();
  initial_res.clear_winning_sources();
  mobilesacn::rpc::ViewLevelsRes res = initial_res;

  // Open connection.
  std::optional<NiceMock<mobilesacn::testing::CrowConnMock>> conn_mock;
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(initial_res.SerializeAsString())).Times(1);
  mobilesacn::rpc::ViewLevels view_levels_handler(etcpal::IpAddr::FromString("127.0.0.1"));
  view_levels_handler.HandleWsOpen(*conn_mock);
  EXPECT_EQ(log_count, 1);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  view_levels_handler.HandleWsClose(*conn_mock, "");

  // Start transmitting.
  test_sources[test_cid.ToString()] = test_source_name;
  test_winning_sources.fill(test_cid.ToString());
  res.clear_sources();
  res.mutable_sources()->insert(test_sources.cbegin(), test_sources.cend());
  res.clear_levels();
  res.mutable_levels()->Add(test_buf.cbegin(), test_buf.cend());
  res.clear_winning_sources();
  for (const auto &cid : test_winning_sources) {
    res.add_winning_sources(cid);
  }
  conn_mock.emplace();
  // The View Levels RPC handler will always send an initial response upon connection before checking for levels.
  EXPECT_CALL(*conn_mock, send_binary(initial_res.SerializeAsString())).Times(1);
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString())).Times(AtLeast(1));
  sacn_transmitter.AddUniverse(test_univ);
  sacn_transmitter.UpdateLevels(test_univ, test_buf.data(), test_buf.size());
  view_levels_handler.HandleWsOpen(*conn_mock);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  view_levels_handler.HandleWsClose(*conn_mock, "");

  // Change some levels.
  // Some vague simulation of real levels.
  for (auto addr = 0, level = 0; addr < test_buf.size(); ++addr, ++level) {
    if (level > 255) {
      level = 0;
    }
    test_buf[addr] = level;
  }
  res.clear_levels();
  res.mutable_levels()->Add(test_buf.cbegin(), test_buf.cend());
  sacn_transmitter.UpdateLevels(test_univ, test_buf.data(), test_buf.size());
  conn_mock.emplace();
  EXPECT_CALL(*conn_mock, send_binary(initial_res.SerializeAsString())).Times(1);
  EXPECT_CALL(*conn_mock, send_binary(res.SerializeAsString())).Times(AtLeast(1));
  view_levels_handler.HandleWsOpen(*conn_mock);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  view_levels_handler.HandleWsClose(*conn_mock, "");

  sacn_transmitter.Shutdown();
}
