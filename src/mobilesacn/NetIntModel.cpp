/**
 * @file NetIntModel.cpp
 *
 * @author Dan Keenan
 * @date 4/9/22
 * @copyright GNU GPLv3
 */

#include "NetIntModel.h"

namespace mobilesacn {

int NetIntModel::rowCount(const QModelIndex &parent) const {
  return etcpal_netint::GetInterfaces().size();
}

int NetIntModel::columnCount(const QModelIndex &parent) const {
  return kColumnCount;
}

QVariant NetIntModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Horizontal) {
    const auto column = static_cast<Column>(section);
    if (role == Qt::DisplayRole) {
      switch (column) {
        case Column::Index:return tr("Index");
        case Column::FriendlyName:return tr("Name");
        case Column::Addr:return tr("Address");
        case Column::Mask:return tr("Subnet Mask");
        case Column::Mac:return tr("MAC Address");
        case Column::IsDefault:return tr("Default");
      }
    }
  }

  return {};
}

QVariant NetIntModel::data(const QModelIndex &index, int role) const {
  const auto column = static_cast<Column>(index.column());
  const auto &iface = etcpal_netint::GetInterfaces().at(index.row());

  if (role == Qt::DisplayRole) {
    if (column == Column::Index) {
      return iface.GetIndex();
    } else if (column == Column::FriendlyName) {
      return QString::fromStdString(iface.GetFriendlyName());
    } else if (column == Column::Addr) {
      return QString::fromStdString(iface.GetAddr().ToString());
    } else if (column == Column::Mask) {
      return QString::fromStdString(iface.GetMask().ToString());
    } else if (column == Column::Mac) {
      return QString::fromStdString(iface.GetMac().ToString());
    }
  } else if (role == Qt::CheckStateRole) {
    if (column == Column::IsDefault) {
      return iface.IsDefault() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
    }
  }

  return {};
}

Qt::ItemFlags NetIntModel::flags(const QModelIndex &index) const {
  const auto column = static_cast<Column>(index.column());
  const auto default_flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
  switch (column) {
    case Column::Index:
    case Column::FriendlyName:
    case Column::Addr:
    case Column::Mask:
    case Column::Mac:return default_flags;
    case Column::IsDefault: return default_flags | Qt::ItemIsUserCheckable;
  }
  return default_flags;
}

etcpal_netint::NetIntInfo &NetIntModel::GetNetIntInfo(int row) const {
  return etcpal_netint::GetInterfaces().at(row);
}

int NetIntModel::GetDefaultRow() const {
  const auto row_count = rowCount(QModelIndex());
  for (int row = 0; row < row_count; ++row) {
    if (GetNetIntInfo(row).IsDefault()) {
      return row;
    }
  }
  return 0;
}

int NetIntModel::GetRowForInterfaceName(const std::string &name) const {
  const auto row_count = rowCount(QModelIndex());
  for (int row = 0; row < row_count; ++row) {
    if (GetNetIntInfo(row).GetFriendlyName() == name) {
      return row;
    }
  }
  return 0;
}

int NetIntListModel::rowCount(const QModelIndex &parent) const {
  return table_model_->rowCount(QModelIndex());
}

QVariant NetIntListModel::data(const QModelIndex &index, int role) const {
  if (role == Qt::DisplayRole) {
    return tr("%1 (%2, %3)")
        .arg(
            table_model_
                ->data(table_model_->index(index.row(), static_cast<int>(NetIntModel::Column::FriendlyName)), role)
                .toString(),
            table_model_->data(table_model_->index(index.row(), static_cast<int>(NetIntModel::Column::Addr)), role)
                .toString(),
            table_model_->data(table_model_->index(index.row(), static_cast<int>(NetIntModel::Column::Mac)), role)
                .toString()
        );
  }

  return {};
}
} // mobilesacn
