/**
 * @file preferred_package.cpp
 *
 * @author Dan Keenan
 * @date 5/13/2026
 * @copyright Apache-2.0
 */

#include "preferred_package.h"
#include <QRegularExpression>

namespace mobilesacn {

QString preferredPackage(const QStringList &filenames)
{
    const auto filenameRegexes = detail::preferredPackageRegexes();

    // Find first matching filename.
    for (const auto &re : filenameRegexes) {
        for (const auto &filename : filenames) {
            if (QRegularExpression(re).match(filename).hasMatch()) {
                return filename;
            }
        }
    }

    return {};
}

} // namespace mobilesacn
