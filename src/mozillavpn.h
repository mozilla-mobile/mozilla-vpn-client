#ifndef MOZILLAVPN_H
#define MOZILLAVPN_H

#include "devicemodel.h"
#include "servercountrymodel.h"
#include "serverdata.h"
#include "user.h"

#include <QList>
#include <QObject>
#include <QPointer>
#include <QSettings>
#include <QNetworkReply>

class Task;

class MozillaVPN final : public QObject
{
    Q_OBJECT

public:
    enum AlertType {
        NoAlert,
        AuthenticationFailedAlert,
        ConnectionFailedAlert,
        LogoutAlert,
        NoConnectionAlert,
    };

    Q_ENUM(AlertType)

private:
    Q_PROPERTY(QString state READ getState NOTIFY stateChanged)
    Q_PROPERTY(QAbstractListModel *serverCountryModel READ serverCountryModel NOTIFY
                   serverCountryModelChanged)
    Q_PROPERTY(QAbstractListModel *deviceModel READ deviceModel NOTIFY deviceModelChanged)
    Q_PROPERTY(int activeDevices READ activeDevices NOTIFY deviceModelChanged)
    Q_PROPERTY(QObject *user READ user)
    Q_PROPERTY(QObject *currentServer READ currentServer)
    Q_PROPERTY(AlertType alert READ alert NOTIFY alertChanged)

public:
    explicit MozillaVPN(QObject *parent = nullptr);
    ~MozillaVPN();

    void initialize(int &argc, char *argv[]);

    QString getState() const { return m_state; }

    const QString &getApiUrl() const { return m_apiUrl; }

    Q_INVOKABLE void authenticate();

    Q_INVOKABLE void cancelAuthentication();

    Q_INVOKABLE void openLink(const QString &linkName);

    Q_INVOKABLE void activate();

    Q_INVOKABLE void deactivate();

    Q_INVOKABLE void removeDevice(const QString &deviceName);

    Q_INVOKABLE void logout();

    Q_INVOKABLE void hideAlert();

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

    ServerData *currentServer() { return &m_serverData; }

    int activeDevices() const;

    User *user() { return &m_user; }

    AlertType alert() const { return m_alert; }

    void errorHandle(QNetworkReply::NetworkError error);

private:
    void setState(const QString &state);

    void scheduleTask(Task *task);
    void maybeRunTask();

signals:
    void stateChanged();
    void deviceModelChanged();
    void serverCountryModelChanged();
    void alertChanged();

private:
    QSettings m_settings;

    QString m_token;
    User m_user;

    ServerData m_serverData;

    DeviceModel m_deviceModel;
    ServerCountryModel m_serverCountryModel;

    QList<QPointer<Task>> m_tasks;
    bool m_task_running = false;

    QString m_state;
    QString m_apiUrl;

    AlertType m_alert = NoAlert;
};

#endif // MOZILLAVPN_H
