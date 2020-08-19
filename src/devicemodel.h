#ifndef DEVICEMODEL_H
#define DEVICEMODEL_H

#include <QAbstractListModel>
#include <QPointer>

#include "userdata.h"

class DeviceModel final : public QAbstractListModel
{
public:
    enum ServerCountryRoles {
        NameRole = Qt::UserRole + 1,
        CurrentOneRole,
    };

    DeviceModel() = default;

    void setUserData(UserData *userData) { m_userData = userData; }

    // QAbstractListModel methods

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &) const override;

    QVariant data(const QModelIndex &index, int role) const override;

private:
    QPointer<UserData> m_userData;
};

#endif // DEVICEMODEL_H
