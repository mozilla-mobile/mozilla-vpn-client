#ifndef SERVERCOUNTRYMODEL_H
#define SERVERCOUNTRYMODEL_H

#include <QAbstractListModel>
#include <QByteArray>
#include <QObject>
#include <QPointer>

class ServerCountry;
class QSettings;

class ServerCountryModel final : public QAbstractListModel
{
public:
    enum ServerCountryRoles {
        NameRole = Qt::UserRole + 1,
        CodeRole,
    };

    ServerCountryModel() = default;

    bool fromSettings(QSettings &settings);

    void fromJson(const QByteArray &data);

    void writeSettings(QSettings &settings);

    // QAbstractListModel methods

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &) const override { return m_countries.length(); }

    QVariant data(const QModelIndex &index, int role) const override;

private:
    void fromJsonInternal();

private:
    QByteArray m_rawJson;

    QList<QPointer<ServerCountry>> m_countries;
};

#endif // SERVERCOUNTRYMODEL_H
