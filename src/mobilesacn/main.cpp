/**
 * @file main.cpp
 *
 * @author Dan Keenan
 * @date 3/25/22
 * @copyright GNU GPLv3
 */

#include "mobilesacn_config.h"
#include "libmobilesacn/Application.h"
#include <lyra/lyra.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include "etcpal_netint/NetIntInfo.h"

/**
 * Store command-line options.
 */
struct ProgramOptions {
  std::string backend_address = "0.0.0.0";
  unsigned int backend_port = 5050;
  std::string sacn_address;
};

/**
 * Setup logging.
 */
void init_logging() {
  // TODO: Support file logging.
  auto log_stdout = spdlog::stdout_color_mt(mobilesacn::config::kProjectName);
  spdlog::set_default_logger(log_stdout);
}

/**
 * Entry point.
 */
int main(int argc, const char *argv[]) {
  ProgramOptions opts;

  return lyra::main(mobilesacn::config::kProjectDescription)
      (lyra::opt(opts.backend_address, "backend address")["--backend-address"](
          fmt::format(
              "The address to listen for mobile connections on. 0.0.0.0 will listen on all interfaces. (Default: {})",
              opts.backend_address)
      ))
      (lyra::opt(opts.backend_port, "backend port")["--backend-port"](
          fmt::format("The port to listen for mobile connections on. (Default: {})",
                      opts.backend_port)
      ))
      (lyra::opt(opts.sacn_address, "sacn address")["--sacn-address"](
          "The interface to transmit/receive sACN traffic on."
      ))
      (argc, argv, [&opts](lyra::main &m) {
        init_logging();

        mobilesacn::Application app(
            {
                .backend_address = opts.backend_address,
                .backend_port = opts.backend_port,
                .sacn_address = etcpal::IpAddr::FromString(opts.sacn_address),
            });

        return app.Run();
      });
}
