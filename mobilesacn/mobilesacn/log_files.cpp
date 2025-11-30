/**
 * @file log_files.cpp
 *
 * @author Dan Keenan
 * @date 5/29/22
 * @copyright GNU GPLv3
 */

#include "log_files.h"
#include <QStandardPaths>
#include <filesystem>
#include <fmt/format.h>
#include "mobilesacn_config.h"

namespace mobilesacn {

std::string get_log_path()
{
    const auto shared_path =
            std::filesystem::path(
                QStandardPaths::writableLocation(QStandardPaths::CacheLocation).toStdString());
    const auto log_path = shared_path / fmt::format("{}.log", config::kProjectName);

    return log_path.string();
}

} // mobilesacn
