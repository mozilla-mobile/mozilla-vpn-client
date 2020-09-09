#ifndef MOZILLAVPN_H
#define MOZILLAVPN_H

#include "controller.h"
#include "devicemodel.h"
#include "keys.h"
#include "releasemonitor.h"
#include "servercountrymodel.h"
#include "serverdata.h"
#include "user.h"

#include <QList>
#include <QNetworkReply>
#include <QObject>
#include <QPointer>
#include <QSettings>
#include <QTimer>

class Task;

class MozillaVPN final : public QObject
{
    Q_OBJECT

public:
    class Holder
    {
    public:
        Holder() { MozillaVPN::createInstance(); }

        ~Holder() { MozillaVPN::deleteInstance(); }
    };

    enum State {
        StateInitialize,
        StateAuthenticating,
        StatePostAuthentication,
        StateMain,
        StateUpdateRequired,
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

    enum LinkType {
        LinkAccount,
        LinkContact,
        LinkFeedback,
        LinkHelpSupport,
        LinkTermsOfService,
        LinkPrivacyPolicy,
        LinkUpdate,
    };
    Q_ENUM(LinkType)

private:
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(AlertType alert READ alert NOTIFY alertChanged)
    Q_PROPERTY(QString versionString READ versionString)
    Q_PROPERTY(bool updateRecommended READ updateRecommended NOTIFY updateRecommendedChanged)
    Q_PROPERTY(bool userAuthenticated READ userAuthenticated NOTIFY userAuthenticationChanged)

public:
    static MozillaVPN *instance();

    State state() const { return m_state; }

    const QString &getApiUrl() const { return m_apiUrl; }

    Q_INVOKABLE void authenticate();

    Q_INVOKABLE void cancelAuthentication();

    Q_INVOKABLE void openLink(LinkType linkType);

    Q_INVOKABLE void removeDevice(const QString &deviceName);

    Q_INVOKABLE void hideAlert() { setAlert(NoAlert); }

    Q_INVOKABLE void hideUpdateRecommendedAlert() { setUpdateRecommended(false); }

    Q_INVOKABLE void postAuthenticationCompleted();

    // Called at the end of the authentication flow. We can continue adding the device
    // if it doesn't exist yet, or we can go to OFF state.
    void authenticationCompleted(const QByteArray &json, const QString &token);

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

    Controller *controller() { return &m_controller; }

    const Keys *keys() const { return &m_keys; }

    void errorHandle(QNetworkReply::NetworkError error);

    void changeServer(const QString &countryCode, const QString &city);

    const QString versionString() const { return QString(APP_VERSION); }

    void logout();

    ConnectionHealth *connectionHealth() { return m_controller.connectionHealth(); }

    bool updateRecommended() const { return m_updateRecommended; }
    void setUpdateRecommended(bool value);

    void forceUpdateState() { setState(StateUpdateRequired); }

    bool userAuthenticated() const { return m_userAuthenticated; }

private:
    explicit MozillaVPN(QObject *parent = nullptr);
    ~MozillaVPN();

    static void createInstance();
    static void deleteInstance();

    void initialize();

    void setAlert(AlertType alert);
    void setState(State state);

    void scheduleTask(Task *task);
    void maybeRunTask();

    void setUserAuthenticated(bool state);

signals:
    void stateChanged();
    void alertChanged();
    void updateRecommendedChanged();
    void userAuthenticationChanged();

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

    State m_state = StateInitialize;
    QString m_apiUrl;

    QTimer m_alertTimer;
    AlertType m_alert = NoAlert;

    ReleaseMonitor m_releaseMonitor;
    bool m_updateRecommended = false;

    bool m_userAuthenticated = false;
};

#endif // MOZILLAVPN_H
