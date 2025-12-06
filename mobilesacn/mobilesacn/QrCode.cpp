/**
 * @file QrCode.cpp
 *
 * @author Dan Keenan
 * @date 4/9/22
 * @copyright GNU GPLv3
 */

#include "QrCode.h"
#include <fmt/format.h>
#include <sstream>
#include <QLabel>
#include <QSvgWidget>
#include <QVBoxLayout>
#include <qrcodegen.hpp>

namespace mobilesacn {

QrCode::QrCode(QWidget *parent) :
    QWidget(parent), svgWidget_(new QSvgWidget(this)), label_(new QLabel(this))
{
    auto *layout = new QVBoxLayout(this);

    svgWidget_->setFixedSize(200, 200);
    layout->addWidget(svgWidget_);

    label_->setTextFormat(Qt::RichText);
    label_->setFixedWidth(svgWidget_->width());
    label_->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    label_->setOpenExternalLinks(true);
    layout->addWidget(label_);
}

void QrCode::updateDisplay()
{
    if (contents_.empty()) {
        svgWidget_->load(QByteArray());
        label_->clear();
        return;
    }

    // Build an SVG that represents the QR Code.
    const auto qr = qrcodegen::QrCode::encodeText(contents_.c_str(), qrcodegen::QrCode::Ecc::HIGH);
    std::ostringstream modules;
    for (int x = 0; x < qr.getSize(); ++x) {
        for (int y = 0; y < qr.getSize(); ++y) {
            if (qr.getModule(x, y)) {
                modules << fmt::format(
                    R"(    <rect x="{x}" y="{y}" width="1" height="1" fill="#000000"/>)"
                    "\n",
                    fmt::arg("x", x),
                    fmt::arg("y", y));
            }
        }
    }

    QByteArray svg;
    svg.reserve(static_cast<int>(modules.tellp()) + 200);
    fmt::format_to(
        std::back_inserter(svg),
        R"(
<svg width="{size}" height="{size}" version="1.1" viewBox="0 0 {size} {size}" xmlns="http://www.w3.org/2000/svg">
  <g stroke="#000000" stroke-width="0.1">
  <rect x="0" y="0" width="{size}" height="{size}" fill="#ffffff"/>
{modules}
  </g>
</svg>
)",
        fmt::arg("size", qr.getSize()),
        fmt::arg("modules", modules.str()));
    svgWidget_->load(svg);

    const auto contents = QString::fromStdString(contents_);
    label_->setText(QString(R"(<a href="%1">%2</a>)").arg(contents.toHtmlEscaped()).arg(contents));
}

} // namespace mobilesacn
