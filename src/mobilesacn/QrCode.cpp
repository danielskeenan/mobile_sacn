/**
 * @file QrCode.cpp
 *
 * @author Dan Keenan
 * @date 4/9/22
 * @copyright GNU GPLv3
 */

#include "QrCode.h"
#include <QLabel>
#include <qrcodegen/qrcodegen.hpp>
#include <fmt/format.h>
#include <sstream>

namespace mobilesacn {

QrCode::QrCode(QWidget *parent) : QSvgWidget(parent) {
  setFixedSize(200, 200);
}

void QrCode::UpdateDisplay() {
  if (contents_.empty()) {
    load(QByteArray());
    return;
  }

  const auto qr = qrcodegen::QrCode::encodeText(contents_.c_str(), qrcodegen::QrCode::Ecc::HIGH);
  std::ostringstream modules;
  for (int x = 0; x < qr.getSize(); ++x) {
    for (int y = 0; y < qr.getSize(); ++y) {
      modules << fmt::format(R"(    <rect x="{x}" y="{y}" width="1" height="1" fill="{fill}"/>)""\n",
                             fmt::arg("x", x),
                             fmt::arg("y", y),
                             fmt::arg("fill", qr.getModule(x, y) ? "#000000" : "#ffffff")
      );
    }
  }

  QByteArray svg;
  svg.reserve(modules.tellp() + 200l);
  fmt::format_to(std::back_inserter(svg), R"(
<svg width="{size}" height="{size}" version="1.1" viewBox="0 0 {size} {size}" xmlns="http://www.w3.org/2000/svg">
  <g stroke-width="0">
{modules}
  </g>
</svg>
)", fmt::arg("size", qr.getSize()), fmt::arg("modules", modules.str()));
  load(svg);
}

} // mobilesacn
