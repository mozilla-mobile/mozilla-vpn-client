#ifndef MOZILLAVPN_H
#define MOZILLAVPN_H

#include "devicemodel.h"
#include "servercountrymodel.h"
#include "serverdata.h"
#include "user.h"
#include "controller.h"
#include "keys.h"

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
    enum State {
        StateInitialize,
        StateAuthenticating,
        StateMain,
    };

    Q_ENUM(State);

    enum AlertType {
        NoAlert,
        AuthenticationFailedAlert,
        ConnectionFailedAlert,
        LogoutAlert,
        NoConnectionAlert,
    };

    Q_ENUM(AlertType)

private:
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(AlertType alert READ alert NOTIFY alertChanged)

public:
    explicit MozillaVPN(QObject *parent = nullptr);
    ~MozillaVPN();

    void initialize(int &argc, char *argv[]);

    State state() const { return m_state; }

    const QString &getApiUrl() const { return m_apiUrl; }

    Q_INVOKABLE void authenticate();

    Q_INVOKABLE void cancelAuthentication();

    Q_INVOKABLE void openLink(const QString &linkName);

    Q_INVOKABLE void removeDevice(const QString &deviceName);

    Q_INVOKABLE void logout();

    Q_INVOKABLE void hideAlert();

    // Called at the end of the authentication flow. We can continue adding the device
    // if it doesn't exist yet, or we can go to OFF state.
    void authenticationCompleted(const QByteArray& json, const QString &token);

    // The device has been added.
    void deviceAdded(const QString &deviceName, const QString &publicKey, const QString &privateKey);

    void deviceRemoved(const QString &deviceName);

    void serversFetched(const QByteArray &serverData);

    void accountChecked(const QByteArray &json);

    QString token() const { return m_token; }

    QAbstractListModel *serverCountryModel() { return &m_serverCountryModel; }

    DeviceModel *deviceModel() { return &m_deviceModel; }

    ServerData *currentServer() { return &m_serverData; }

    const QList<Server> getServers() const;

    User *user() { return &m_user; }

    AlertType alert() const { return m_alert; }

    Controller* controller() { return &m_controller; }

    const Keys* keys() const { return &m_keys; }

    void errorHandle(QNetworkReply::NetworkError error);

    void changeServer(const QString &countryCode, const QString &city);

private:
    void setState(State state);

    void scheduleTask(Task *task);
    void maybeRunTask();

signals:
    void stateChanged();
    void alertChanged();

private:
    QSettings m_settings;

    QString m_token;
    User m_user;

    ServerData m_serverData;

    Controller m_controller;

    DeviceModel m_deviceModel;
    ServerCountryModel m_serverCountryModel;

    Keys m_keys;

    QList<QPointer<Task>> m_tasks;
    bool m_task_running = false;

    State m_state;
    QString m_apiUrl;

    AlertType m_alert = NoAlert;
};

#endif // MOZILLAVPN_H
