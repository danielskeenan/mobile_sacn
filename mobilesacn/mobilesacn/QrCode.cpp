/**
 * @file QrCode.cpp
 *
 * @author Dan Keenan
 * @date 4/9/22
 * @copyright GNU GPLv3
 */

#include "QrCode.h"
#include <QLabel>
#include <qrcodegen.hpp>
#include <fmt/format.h>
#include <sstream>

namespace mobilesacn {

QrCode::QrCode(QWidget* parent)
    : QSvgWidget(parent)
{
    setFixedSize(200, 200);
}

void QrCode::updateDisplay()
{
    if (contents_.empty()) {
        load(QByteArray());
        return;
    }

    // Build an SVG that represents the QR Code.
    const auto qr = qrcodegen::QrCode::encodeText(contents_.c_str(), qrcodegen::QrCode::Ecc::HIGH);
    std::ostringstream modules;
    for (int x = 0; x < qr.getSize(); ++x) {
        for (int y = 0; y < qr.getSize(); ++y) {
            if (qr.getModule(x, y)) {
                modules << fmt::format(
                    R"(    <rect x="{x}" y="{y}" width="1" height="1" fill="#000000"/>)""\n",
                    fmt::arg("x", x),
                    fmt::arg("y", y)
                );
            }
        }
    }

    QByteArray svg;
    svg.reserve(static_cast<int>(modules.tellp()) + 200);
    fmt::format_to(std::back_inserter(svg), R"(
<svg width="{size}" height="{size}" version="1.1" viewBox="0 0 {size} {size}" xmlns="http://www.w3.org/2000/svg">
  <g stroke="#000000" stroke-width="0.1">
  <rect x="0" y="0" width="{size}" height="{size}" fill="#ffffff"/>
{modules}
  </g>
</svg>
)", fmt::arg("size", qr.getSize()), fmt::arg("modules", modules.str()));
    load(svg);
}

} // mobilesacn
