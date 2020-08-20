#ifndef MOZILLAVPN_H
#define MOZILLAVPN_H

#include "devicemodel.h"
#include "servercountrymodel.h"
#include "user.h"

#include <QList>
#include <QObject>
#include <QPointer>
#include <QSettings>

class Task;

class MozillaVPN final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString state READ getState NOTIFY stateChanged)
    Q_PROPERTY(QAbstractListModel *serverCountryModel READ serverCountryModel NOTIFY
                   serverCountryModelChanged)
    Q_PROPERTY(QAbstractListModel *deviceModel READ deviceModel NOTIFY deviceModelChanged)
    Q_PROPERTY(int activeDevices READ activeDevices NOTIFY deviceModelChanged)
    Q_PROPERTY(int maxDevices READ maxDevices NOTIFY deviceModelChanged)

public:
    explicit MozillaVPN(QObject *parent = nullptr);
    ~MozillaVPN();

    void initialize(int &argc, char *argv[]);

    QString getState() const { return m_state; }

    const QString &getApiUrl() const { return m_apiUrl; }

    Q_INVOKABLE void authenticate();

    Q_INVOKABLE void openLink(const QString &linkName);

    Q_INVOKABLE void activate();

    Q_INVOKABLE void removeDevice(const QString &deviceName);

    // Called at the end of the authentication flow. We can continue adding the device
    // if it doesn't exist yet, or we can go to OFF state.
    void authenticationCompleted(QJsonObject &userObj, const QString &token);

    // The device has been added.
    void deviceAdded(const QString &deviceName, const QString &publicKey, const QString &privateKey);

    void deviceRemoved(const QString &deviceName);

    void serversFetched(const QByteArray &serverData);

    void accountChecked(QJsonObject &userObj);

    QString token() const { return m_token; }

    QAbstractListModel *serverCountryModel() { return &m_serverCountryModel; }

    DeviceModel *deviceModel() { return &m_deviceModel; }

    int activeDevices() const;

    int maxDevices() const;

private:
    void setState(const QString &state);

    void scheduleTask(Task *task);
    void maybeRunTask();

signals:
    void stateChanged();
    void deviceModelChanged();
    void serverCountryModelChanged();

private:
    QSettings m_settings;

    QString m_token;
    User m_user;

    DeviceModel m_deviceModel;
    ServerCountryModel m_serverCountryModel;

    QList<QPointer<Task>> m_tasks;
    bool m_task_running = false;

    QString m_state;
    QString m_apiUrl;
};

#endif // MOZILLAVPN_H
