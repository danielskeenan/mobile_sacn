/**
 * @file ClientTableModel.cpp
 *
 * @author Dan Keenan
 * @date 5/9/26
 * @copyright GPL-3.0
 */

#include "ClientTableModel.h"

namespace mobilesacn {

ClientTableModel::ClientTableModel(Application *application, QObject *parent) :
    QAbstractTableModel(parent)
{
    connect(application, &Application::handlerStarted, this, &ClientTableModel::handlerStarted);
    connect(application, &Application::handlerStopped, this, &ClientTableModel::handlerStopped);
}

int ClientTableModel::rowCount(const QModelIndex &parent) const
{
    return clients_.size();
}

int ClientTableModel::columnCount(const QModelIndex &parent) const
{
    return kColumnCount;
}

QVariant ClientTableModel::data(const QModelIndex &index, int role) const
{
    const auto column = static_cast<Column>(index.column());
    const auto client = clients_.at(index.row());

    if (role == Qt::DisplayRole) {
        if (column == Column::Address) {
            return client.address.toString();
        } else if (column == Column::Handler) {
            return client.handler;
        }
    }

    return {};
}

QVariant ClientTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        const auto column = static_cast<Column>(section);
        if (role == Qt::DisplayRole) {
            switch (column) {
            case Column::Address:
                return tr("IP Address");
            case Column::Handler:
                return tr("Function");
            }
        }
    }

    return {};
}

void ClientTableModel::handlerStarted(const QString &displayName, const QHostAddress &clientAddress)
{
    beginResetModel();
    clients_.emplace_back(clientAddress, displayName);
    std::ranges::sort(clients_, [](const Client &lhs, const Client &rhs) {
        // Client address.
        if (lhs.address != rhs.address) {
            if (lhs.address.protocol() != rhs.address.protocol()) {
                return lhs.address.protocol() < rhs.address.protocol();
            }
            return lhs.address.toString() < rhs.address.toString();
        }

        // Handler name.
        return lhs.handler < rhs.handler;
    });
    endResetModel();
}

void ClientTableModel::handlerStopped(const QString &displayName, const QHostAddress &clientAddress)
{
    for (int ix = 0; ix < clients_.size(); ++ix) {
        const auto &client = clients_.at(ix);
        if (client.address == clientAddress && client.handler == displayName) {
            beginRemoveRows({}, ix, ix);
            clients_.erase(clients_.begin() + ix);
            endRemoveRows();
        }
    }
}

} // namespace mobilesacn
