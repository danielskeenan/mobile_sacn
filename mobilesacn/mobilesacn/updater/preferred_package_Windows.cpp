/**
 * @file preferred_package_Windows.cpp
 *
 * @author Dan Keenan
 * @date 5/13/2026
 * @copyright Apache-2.0
 */

#include "mobilesacn_config.h"
#include "preferred_package.h"
#include <Windows.h>
#include <filesystem>
#include <msi.h>
#include <QApplication>

namespace mobilesacn::detail {
QStringList preferredPackageRegexes()
{
    QStringList filenameRegexes;
    const std::filesystem::path appPath(qApp->applicationFilePath().toStdString());

    // Get MSI product code (which changes each release) from upgrade code (always the same).
    std::wstring productCode(MAX_GUID_CHARS, L'\0');
    for (DWORD productIx = 0;; ++productIx) {
        auto ret = MsiEnumRelatedProducts(
            config::kProjectMsiUpgradeCode, 0, productIx, productCode.data());
        if (ret != ERROR_SUCCESS) {
            break;
        }
        // Check to see we are running from this product.
        std::wstring componentId(MAX_GUID_CHARS, L'\0');
        for (DWORD componentIx = 0;; ++componentIx) {
            ret = MsiEnumComponents(componentIx, componentId.data());
            if (ret != ERROR_SUCCESS) {
                break;
            }
            DWORD pathBufSize = 0;
            const auto installState
                = MsiGetComponentPath(productCode.data(), componentId.data(), nullptr, &pathBufSize);
            if (installState != INSTALLSTATE_LOCAL) {
                continue;
            }
            std::wstring pathBuf(pathBufSize, L'\0');
            ++pathBufSize;
            MsiGetComponentPath(productCode.data(), componentId.data(), pathBuf.data(), &pathBufSize);
            try {
                const std::filesystem::path componentPath(pathBuf);
                if (std::filesystem::equivalent(appPath, std::filesystem::path(pathBuf))) {
                    // This program came from an MSI.
                    filenameRegexes.emplace_back(QStringLiteral("-Windows\\.msi$"));
                    break;
                }
            } catch (const std::exception &) {
                // MsiGetComponentPath() may return non-paths which will make the std::filesystem::path ctor throw.
                // That exception is implementation-defined so need the broad catch here.
                // Since an invalid path certainly is not equal to our app path, skip it and continue.
                continue;
            }
        }
    }

    filenameRegexes.emplace_back(QStringLiteral("-Windows\\.zip$"));

    return filenameRegexes;
}
} // namespace mobilesacn::detail
