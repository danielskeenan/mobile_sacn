/**
 * @file preferred_package.h
 *
 * @author Dan Keenan
 * @date 5/13/2026
 * @copyright Apache-2.0
 */

#ifndef MOBILESACN_MOBILESACN_UPDATER_PREFERRED_PACKAGE_H
#define MOBILESACN_MOBILESACN_UPDATER_PREFERRED_PACKAGE_H

#include <QStringList>

namespace mobilesacn {

/**
 * Find the correct package for this system.
 *
 * @param filenames List of available package filenames.
 * @return The filename to use, or an empty string if one could not be found.
 */
QString preferredPackage(const QStringList &filenames);

namespace detail {
/**
 * Platform-specific regexes to match against filenames.
 */
QStringList preferredPackageRegexes();
} // namespace detail
} // namespace mobilesacn

#endif //MOBILESACN_MOBILESACN_UPDATER_PREFERRED_PACKAGE_H
