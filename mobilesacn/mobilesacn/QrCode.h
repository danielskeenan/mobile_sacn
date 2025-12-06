/**
 * @file QrCode.h
 *
 * @author Dan Keenan
 * @date 4/9/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_SRC_MOBILESACN_QRCODE_H_
#define MOBILE_SACN_SRC_MOBILESACN_QRCODE_H_

#include <QWidget>

class QSvgWidget;
class QLabel;

namespace mobilesacn {
class QrCode : public QWidget
{
    Q_OBJECT

public:
    explicit QrCode(QWidget *parent = nullptr);

    [[nodiscard]] const std::string &getContents() const { return contents_; }

    void setContents(const std::string &contents)
    {
        contents_ = contents;
        updateDisplay();
    }

    void clear()
    {
        contents_.clear();
        updateDisplay();
    };

private:
    QSvgWidget *svgWidget_;
    QLabel *label_;
    std::string contents_;

    void updateDisplay();
};
} // namespace mobilesacn

#endif //MOBILE_SACN_SRC_MOBILESACN_QRCODE_H_
