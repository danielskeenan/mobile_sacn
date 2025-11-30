/**
 * @file NetIntModel.h
 *
 * @author Dan Keenan
 * @date 4/9/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_SRC_MOBILESACN_NETINTMODEL_H_
#define MOBILE_SACN_SRC_MOBILESACN_NETINTMODEL_H_

#include <QAbstractListModel>
#include <etcpal/cpp/netint.h>

namespace mobilesacn {
/**
 * ItemModel for Network Interfaces.
 */
class NetIntModel : public QAbstractTableModel {
    Q_OBJECT

  public:
    enum class Column {
      Index,
      FriendlyName,
      Addr,
      Mask,
      Mac,
      IsDefault,
    };
    static const auto kColumnCount = static_cast<std::underlying_type_t<Column>>(Column::IsDefault) + 1;

    explicit NetIntModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;
    [[nodiscard]] const etcpal::NetintInfo &getNetIntInfo(int row) const {
        return netints_.at(row);
    }
    [[nodiscard]] int getDefaultRow() const;
    [[nodiscard]] int getRowForInterfaceName(const std::string &name) const;

private:
    std::vector<etcpal::NetintInfo> netints_;
};

/**
 * List version of NetIntModel.
 */
class NetIntListModel : public QAbstractListModel {
    Q_OBJECT

  public:
    explicit NetIntListModel(QObject *parent = nullptr)
      : QAbstractListModel(parent), table_model_(new NetIntModel(this)) {
    }

    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    [[nodiscard]] inline const etcpal::NetintInfo &GetNetIntInfo(int row) const {
      return table_model_->getNetIntInfo(row);
    }

    [[nodiscard]] int GetDefaultRow() const {
      return table_model_->getDefaultRow();
    }

    [[nodiscard]] int GetRowForInterfaceName(const std::string &name) const {
      return table_model_->getRowForInterfaceName(name);
    }

  private:
    NetIntModel *table_model_;
};
} // mobilesacn

#endif //MOBILE_SACN_SRC_MOBILESACN_NETINTMODEL_H_
