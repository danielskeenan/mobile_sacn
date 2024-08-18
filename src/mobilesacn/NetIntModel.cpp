/**
 * @file NetIntModel.cpp
 *
 * @author Dan Keenan
 * @date 4/9/22
 * @copyright GNU GPLv3
 */

#include "NetIntModel.h"

namespace mobilesacn {

int NetIntModel::rowCount(const QModelIndex& parent) const
{
    return etcpal::netint::GetInterfaces()->size();
}

int NetIntModel::columnCount(const QModelIndex& parent) const
{
    return kColumnCount;
}

QVariant NetIntModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        const auto column = static_cast<Column>(section);
        if (role == Qt::DisplayRole) {
            switch (column) {
            case Column::Index:
                return tr("Index");
            case Column::FriendlyName:
                return tr("Name");
            case Column::Addr:
                return tr("Address");
            case Column::Mask:
                return tr("Subnet Mask");
            case Column::Mac:
                return tr("MAC Address");
            case Column::IsDefault:
                return tr("Default");
            }
        }
    }

    return {};
}

QVariant NetIntModel::data(const QModelIndex& index, int role) const
{
    const auto column = static_cast<Column>(index.column());
    const auto& iface = getNetIntInfo(index.row());

    if (role == Qt::DisplayRole) {
        if (column == Column::Index) {
            return iface.index().value();
        } else if (column == Column::FriendlyName) {
            return QString::fromStdString(iface.friendly_name());
        } else if (column == Column::Addr) {
            return QString::fromStdString(iface.addr().ToString());
        } else if (column == Column::Mask) {
            return QString::fromStdString(iface.mask().ToString());
        } else if (column == Column::Mac) {
            return QString::fromStdString(iface.mac().ToString());
        }
    } else if (role == Qt::CheckStateRole) {
        if (column == Column::IsDefault) {
            return iface.is_default() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
        }
    }

    return {};
}

Qt::ItemFlags NetIntModel::flags(const QModelIndex& index) const
{
    const auto column = static_cast<Column>(index.column());
    constexpr auto defaultFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled |
            Qt::ItemNeverHasChildren;
    switch (column) {
    case Column::Index:
    case Column::FriendlyName:
    case Column::Addr:
    case Column::Mask:
    case Column::Mac:
        return defaultFlags;
    case Column::IsDefault:
        return defaultFlags | Qt::ItemIsUserCheckable;
    }
    return defaultFlags;
}

etcpal::NetintInfo& NetIntModel::getNetIntInfo(int row) const
{
    return etcpal::netint::GetInterfaces().value().at(row);
}

int NetIntModel::getDefaultRow() const
{
    const auto row_count = rowCount(QModelIndex());
    for (int row = 0; row < row_count; ++row) {
        if (getNetIntInfo(row).is_default()) {
            return row;
        }
    }
    return 0;
}

int NetIntModel::getRowForInterfaceName(const std::string& name) const
{
    const auto row_count = rowCount(QModelIndex());
    for (int row = 0; row < row_count; ++row) {
        if (getNetIntInfo(row).friendly_name() == name) {
            return row;
        }
    }
    return 0;
}

int NetIntListModel::rowCount(const QModelIndex& parent) const
{
    return table_model_->rowCount(QModelIndex());
}

QVariant NetIntListModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole) {
        const auto friendlyName = table_model_->data(
                    table_model_->index(
                        index.row(), static_cast<int>(NetIntModel::Column::FriendlyName)), role)
                .toString();
        const auto ipAddress = table_model_->data(
                    table_model_->index(
                        index.row(), static_cast<int>(NetIntModel::Column::Addr)), role)
                .toString();
        const auto macAddress = table_model_->data(
                    table_model_->index(
                        index.row(), static_cast<int>(NetIntModel::Column::Mac)), role)
                .toString();
        return tr("%1 (%2, %3)")
                .arg(friendlyName, ipAddress, macAddress);
    }

    return {};
}
} // mobilesacn
