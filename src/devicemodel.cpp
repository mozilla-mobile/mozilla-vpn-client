#include "devicemodel.h"
#include "userdata.h"

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[CurrentOneRole] = "currentOne";
    return roles;
}

int DeviceModel::rowCount(const QModelIndex &) const
{
    if (!m_userData) {
        return 0;
    }

    return m_userData->devices().count();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case NameRole:
        return QVariant(m_userData->devices().at(index.row()).deviceName());

    case CurrentOneRole:
        return QVariant(m_userData->devices().at(index.row()).hasPrivateKey());

    default:
        return QVariant();
    }
}
