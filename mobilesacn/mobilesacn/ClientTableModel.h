/**
 * @file ClientTableModel.h
 *
 * @author Dan Keenan
 * @date 5/9/26
 * @copyright GPL-3.0
 */

#ifndef MOBILESACN_MOBILESACN_CLIENTTABLEMODEL_H
#define MOBILESACN_MOBILESACN_CLIENTTABLEMODEL_H

#include "mobilesacn/libmobilesacn/Application.h"
#include <QAbstractTableModel>
#include <QHostAddress>

namespace mobilesacn {

/**
 * Model for client table in MainWindow.
 */
class ClientTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum class Column {
        Address,
        Handler,
    };
    static constexpr auto kColumnCount = static_cast<std::underlying_type_t<Column>>(Column::Handler)
                                         + 1;

    explicit ClientTableModel(Application *application, QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QVariant headerData(
        int section, Qt::Orientation orientation, int role) const override;

private:
    struct Client
    {
        QHostAddress address;
        QString handler;
    };
    std::vector<Client> clients_;

private Q_SLOTS:
    void handlerStarted(const QString &displayName, const QHostAddress &clientAddress);
    void handlerStopped(const QString &displayName, const QHostAddress &clientAddress);
};

} // namespace mobilesacn

#endif //MOBILESACN_MOBILESACN_CLIENTTABLEMODEL_H
