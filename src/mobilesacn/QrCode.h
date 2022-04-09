/**
 * @file QrCode.h
 *
 * @author Dan Keenan
 * @date 4/9/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_SRC_MOBILESACN_QRCODE_H_
#define MOBILE_SACN_SRC_MOBILESACN_QRCODE_H_

#include <QSvgWidget>

namespace mobilesacn {

class QrCode : public QSvgWidget {
 Q_OBJECT
 public:
  explicit QrCode(QWidget *parent = nullptr);

  [[nodiscard]] const std::string &GetContents() const {
    return contents_;
  }

  void SetContents(const std::string &contents) {
    contents_ = contents;
    UpdateDisplay();
  }

  void Clear() {
    contents_.clear();
    UpdateDisplay();
  };

 private:
  std::string contents_;

  void UpdateDisplay();
};

} // mobilesacn

#endif //MOBILE_SACN_SRC_MOBILESACN_QRCODE_H_
